
//A voir pour les inclusions
#include <mysql.h>
#include <mysql/client_plugin.h>
#include <mysqld_error.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  
    //Let's init...
    MYSQL *mysql;
    MY_INIT();
    if (mysql_server_init(0, NULL, NULL)) return 1;
    mysql = client_connect(0, MYSQL_PROTOCOL_DEFAULT, 1);

    //Null character in the middle of data ; splitting them to make several queries
    //Here is the test
    std:string dataAsString(Data, Data + Size);
    int rc;
    rc = mysql_query(mysql, dataAsString);
    myquery(rc);
    
    
    //Let's stop...
    client_disconnect(mysql);
    mysql_server_end();
    my_end(0);

  return 0;
}
