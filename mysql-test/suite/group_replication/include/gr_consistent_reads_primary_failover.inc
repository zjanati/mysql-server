################################################################################
# Validate the behaviour of consistent transactions during primary failover.
#
# Test:
# 00. The test requires three servers: server1,server2,server3
# 01. Create a table on the group.
# 02. Lock table t1 on server3
# 03. Perform a trasanction T1 on server1 on table t1 with 'AFTER'
# 04. Try to query table t1 (Transaction T2) on server2, it
#     will be blocked since T1 is prepared but not committed.
# 05. Crash server1. Ensure that server2 is elected as primary.
# 06. Unlock tables on server3. Check the transaction from
#     server1 succeeds now.
# 07. Check that server2 has not applied T1.
# 08. Wait for T2 to timeout, only then T1 will be committed
#     and externalized on server2.
# 09. Clean up.
################################################################################

#
# ==== Usage ====
#
# --let $transaction_T2_consistency= ...
# --let $transaction_T2_error= ERROR_NAME
#
#
# Parameters:
#
#   $transaction_T2_consistency
#     Set the given consistency guarantee to transaction T2.
#   $transaction_T2_error
#     Expect this error from T2 outcome.
#
################################################################################

--source include/big_test.inc
# Don't test this under valgrind, memory leaks will occur
--source include/not_valgrind.inc
--source include/force_restart.inc
--source include/have_group_replication_plugin.inc
--let $rpl_group_replication_single_primary_mode=1
--let $rpl_server_count= 3
--let $rpl_skip_group_replication_start= 1
--source include/group_replication.inc


--echo
--echo ############################################################
--echo # 01. Create a table on the group.
--let $rpl_connection_name= server1
--source include/rpl_connection.inc
--source include/start_and_bootstrap_group_replication.inc
CREATE TABLE t1 (c1 INT NOT NULL PRIMARY KEY);

--let $rpl_connection_name= server2
--source include/rpl_connection.inc
SET SESSION sql_log_bin= 0;
call mtr.add_suppression("Error on session '.*' while waiting for its dependencies to complete commit.");
call mtr.add_suppression("Error releasing transaction '.*' for execution on session '.*' after its dependencies did complete commit.");
call mtr.add_suppression("Error waiting for group executed transactions commit on session '.*'.");
SET SESSION sql_log_bin= 1;

--source include/start_group_replication.inc
--let $member2_uuid= query_get_value(SELECT @@SERVER_UUID, @@SERVER_UUID, 1)

--let $rpl_connection_name= server3
--source include/rpl_connection.inc
--source include/start_group_replication.inc


--echo
--echo ############################################################
--echo # 02. Lock table t1 on server3
--let $rpl_connection_name= server3
--source include/rpl_connection.inc
LOCK TABLE t1 READ;


--echo
--echo ############################################################
--echo # 03. Perform a transaction T1 on server1 on table t1 with 'AFTER'
--let $rpl_connection_name= server1
--source include/rpl_connection.inc

SET @@SESSION.group_replication_consistency='AFTER';
--send INSERT INTO t1 VALUES(11)

# Check that T1 is blocked on server1, server2 for server3's ack.
--let $rpl_connection_name= server_1
--source include/rpl_connection.inc
--let $wait_condition=SELECT COUNT(*)=1 FROM information_schema.processlist WHERE user='root' AND state='waiting for handler commit'
--source include/wait_condition.inc

--let $rpl_connection_name= server2
--source include/rpl_connection.inc
--let $wait_condition=SELECT COUNT(*)=1 FROM information_schema.processlist WHERE user='system user' AND state='waiting for handler commit'
--source include/wait_condition.inc


--echo
--echo ############################################################
--echo # 04. Try to query table t1 (Transaction T2) on server2, it
--echo #     will be blocked since T1 is prepared but not committed.
--let $rpl_connection_name= server2
--source include/rpl_connection.inc

SET @@SESSION.wait_timeout = 30;
if ($transaction_T2_consistency)
{
  --eval SET @@SESSION.group_replication_consistency= '$transaction_T2_consistency'
}
--send SELECT * FROM test.t1

--let $rpl_connection_name= server_2
--source include/rpl_connection.inc
--let $wait_condition=SELECT COUNT(*)=1 FROM information_schema.processlist WHERE user='system user' AND state='waiting for handler commit'
--source include/wait_condition.inc

--let $wait_condition=SELECT COUNT(*)=1 FROM information_schema.processlist WHERE user='root' AND state='Executing hook on transaction begin.'
--source include/wait_condition.inc


--echo
--echo ############################################################
--echo # 05. Crash server1. Ensure that server2 is elected as primary.
--let $rpl_connection_name= server_1
--source include/rpl_connection.inc
--exec echo "restart" > $MYSQLTEST_VARDIR/tmp/mysqld.1.expect
--shutdown_server 0
--source include/wait_until_disconnected.inc

--let $rpl_connection_name= server1
--source include/rpl_connection.inc
--error CR_SERVER_LOST
--reap

--let $rpl_connection_name= server_2
--source include/rpl_connection.inc
--let $wait_condition= SELECT MEMBER_ROLE="PRIMARY" FROM performance_schema.replication_group_members where member_id='$member2_uuid'
--source include/wait_condition.inc

# Ensure that transaction from server2 is still blocked, also "select" on a parallel session is blocked.
--let $wait_condition=SELECT COUNT(*)=1 FROM information_schema.processlist WHERE user='system user' AND state='waiting for handler commit'
--source include/wait_condition.inc

--let $wait_condition=SELECT COUNT(*)=1 FROM information_schema.processlist WHERE user='root' AND state='Executing hook on transaction begin.'
--source include/wait_condition.inc


--echo
--echo ############################################################
--echo # 06. Unlock tables on server3. Check the transaction from
--echo #     server1 succeeds now.
--let $rpl_connection_name= server3
--source include/rpl_connection.inc
UNLOCK TABLES;

--let $assert_text= 'Table t1 must have 1 entry.'
--let $assert_cond= [SELECT COUNT(*) as count FROM test.t1,count, 1] = 1
--source include/assert.inc


--echo
--echo #############################################################
--echo # 07. Check that server2 has not applied T1.
--let $rpl_connection_name= server_2
--source include/rpl_connection.inc
--let $wait_condition=SELECT COUNT(*)=1 FROM information_schema.processlist WHERE user='system user' AND state='waiting for handler commit'
--source include/wait_condition.inc

--echo
--echo ############################################################
--echo # 08. Wait for T2 to timeout, only then T1 will be committed
--echo #     and externalized on server2.
--let $rpl_connection_name= server2
--source include/rpl_connection.inc
--error $transaction_T2_error
--reap

--let $assert_text= 'Table t1 must have 1 entry.'
--let $assert_cond= [SELECT COUNT(*) as count FROM test.t1,count, 1] = 1
--source include/assert.inc


--echo
--echo ############################################################
--echo # 9. Clean up.
--let $rpl_connection_name= server1
--source include/rpl_connection.inc
--let $rpl_server_number= 1
--source include/rpl_reconnect.inc

--let $assert_text= 'Table t1 must not have 1 entry.'
--let $assert_cond= [SELECT COUNT(*) as count FROM test.t1,count, 1] = 0
--source include/assert.inc

SET SESSION sql_log_bin= 0;
DROP TABLE t1;
SET SESSION sql_log_bin= 1;

--let $rpl_connection_name= server2
--source include/rpl_connection.inc
DROP TABLE t1;

--source include/group_replication_end.inc
