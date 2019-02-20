# Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

MYSQL_ADD_COMPONENT(validate_password
  validate_password_imp.cc
  MODULE
  LINK_LIBRARIES library_mysys)

MYSQL_ADD_EXECUTABLE(validate_password_fuzz
        validate_password_fuzz.cc
        validate_password_imp.cc
        validate_password_imp.h
        )
TARGET_LINK_LIBRARIES(validate_password_fuzz library_mysys)
TARGET_LINK_LIBRARIES(validate_password_fuzz mysqlclient)
TARGET_LINK_LIBRARIES(validate_password_fuzz FuzzingEngine)
TARGET_LINK_LIBRARIES(validate_password_fuzz
  sql_main sql_gis binlog rpl master slave sql_main sql_dd sql_gis mysys mysys_ssl
  binlogevents_static ${ICU_LIBRARIES})
