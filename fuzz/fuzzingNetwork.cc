#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <mysql.h>
#include <mysql/client_plugin.h>
#include <mysqld_error.h>
#include <mysql_socket.h>

using namespace std;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  MYSQL mysql;
  mysql_init(&mysql);
  if (!mysql_real_connect(&mysql,"localhost","root","root","",0,NULL,0)) {
      printf("Can't connect!\n");
      return 1;
  }
  mysql_close(&mysql);
}

