#include "webservice.h"
#include "packages/json/json.h"
#include "packages/mariadb/mariadb.h"
#include <assert.h>

DUDA_REGISTER("MariaDB dthread support demonstration", "mariadb_dthread");

static inline void print_header(duda_request_t *dr)
{
    response->printf(dr, "\n\
<!DOCTYPE html>\n\
<html lang=\"en\">\n\
<head>\n\
  <meta charset=\"UTF-8\">\n\
  <title>MariaDB Package Demo</title>\n\
  <link rel=\"stylesheet\" href=\"//netdna.bootstrapcdn.com/bootstrap/3.0.0/css/bootstrap.min.css\">\n\
  <style type=\"text/css\">\n\
    body {\n\
      position: relative;\n\
      padding-top: 80px;\n\
    }\n\
  </style>\n\
</head>\n\
<body>\n\
  <div class=\"navbar navbar-inverse navbar-fixed-top\">\n\
    <div class=\"container\">\n\
      <a href=\"#\" class=\"navbar-brand\">MariaDB Package</a>\n\
      <ul class=\"nav navbar-nav\">\n\
        <li id=\"home\">\n\
          <a href=\"/mariadb_dthread/mariadb/home/\">Home</a>\n\
        </li>\n\
        <li id=\"dashboard\">\n\
          <a href=\"/mariadb_dthread/mariadb/dashboard/\">Dashboard</a>\n\
        </li>\n\
      </ul>\n\
    </div>\n\
  </div>");
}

static inline void print_footer(duda_request_t *dr, const char *js)
{
    response->printf(dr, "\n\
  <script src=\"//ajax.googleapis.com/ajax/libs/jquery/1.10.1/jquery.min.js\"></script>\n\
  <script src=\"//netdna.bootstrapcdn.com/bootstrap/3.0.0/js/bootstrap.min.js\"></script>");
    if (js) {
        response->printf(dr, js);
    }
    response->printf(dr, "\n\
</body>\n\
</html>");
}

void list_databases(void *data)
{
    duda_request_t *dr = data;
    assert(dr);
    json_t *db_array = json->create_array();
    response->http_status(dr, 200);
    response->http_header_n(dr, "Content-Type: application/json", 30);
    response->wait(dr);

    mariadb_conn_t *conn = mariadb->create_conn(dr, "root", "db", "localhost",
            NULL, 0, "/run/mysqld/mysqld.sock", 0);
    if (!conn) {
        response->printf(dr, "No Connection Available\n");
        response->cont(dr);
        response->end(dr, NULL);
    }
    int status = mariadb->dthread_connect(conn);
    if (status != MARIADB_OK) {
        response->printf(dr, "Can't connect to MariaDB server\n");
        response->cont(dr);
        response->end(dr, NULL);
    }
    mariadb->dthread_query(conn, "SET NAMES 'UTF8'");
    mariadb_result_t *result = mariadb->dthread_query(conn, "SHOW DATABASES");
    assert(result);
    char **row = NULL;
    int error;
    json_t *item;
    while (1) {
        row = mariadb->dthread_get_row(conn, result, &error);
        if (!row) {
            if (error) {
                response->cont(dr);
                response->end(dr, NULL);
            } else {
                break;
            }
        }
        item = json->create_string(row[0]);
        json->add_to_array(db_array, item);
    }
    mariadb->dthread_disconnect(conn);
    char *encoded_str = json->print_gc(dr, db_array);
    response->printf(dr, "%s", encoded_str);
    response->cont(dr);
    response->end(dr, NULL);
}

void cb_list_databases(duda_request_t *dr)
{
    int id = dthread->create(list_databases, dr);
    dthread->resume(id);
}

void list_tables(void *data)
{
    duda_request_t *dr = data;
    assert(dr);
    json_t *tb_array = json->create_array();
    char *db = param->get(dr, 0);
    char use_db_query[128];

    response->http_status(dr, 200);
    response->http_header_n(dr, "Content-Type: application/json", 30);
    response->wait(dr);

    mariadb_conn_t *conn = mariadb->create_conn(dr, "root", "db", "localhost",
            NULL, 0, "/run/mysqld/mysqld.sock", 0);
    if (!conn) {
        response->printf(dr, "No Connection Available\n");
        response->cont(dr);
        response->end(dr, NULL);
    }
    int status = mariadb->dthread_connect(conn);
    if (status != MARIADB_OK) {
        response->printf(dr, "Can't connect to MariaDB server\n");
        response->cont(dr);
        response->end(dr, NULL);
    }
    mariadb->dthread_query(conn, "SET NAMES 'UTF8'");
    sprintf(use_db_query, "USE %s", db);
    mariadb->dthread_query(conn, use_db_query);
    mariadb_result_t *result = mariadb->dthread_query(conn, "SHOW TABLES");
    assert(result);
    char **row = NULL;
    int error;
    json_t *item;
    while (1) {
        row = mariadb->dthread_get_row(conn, result, &error);
        if (!row) {
            if (error) {
                response->cont(dr);
                response->end(dr, NULL);
            } else {
                break;
            }
        }
        item = json->create_string(row[0]);
        json->add_to_array(tb_array, item);
    }
    mariadb->dthread_disconnect(conn);
    char *encoded_str = json->print_gc(dr, tb_array);
    response->printf(dr, "%s", encoded_str);
    response->cont(dr);
    response->end(dr, NULL);
}

void cb_list_tables(duda_request_t *dr)
{
    int id = dthread->create(list_tables, dr);
    dthread->resume(id);
}

void row_nums(void *data)
{
    duda_request_t *dr = data;
    assert(dr);
    json_t *row_nums = json->create_array();
    char *db = param->get(dr, 0);
    char *table = param->get(dr, 1);
    char use_db_query[128];
    char count_query[128];

    response->http_status(dr, 200);
    response->http_header_n(dr, "Content-Type: application/json", 30);
    response->wait(dr);

    mariadb_conn_t *conn = mariadb->create_conn(dr, "root", "db", "localhost",
            NULL, 0, "/run/mysqld/mysqld.sock", 0);
    if (!conn) {
        response->printf(dr, "No Connection Available\n");
        response->cont(dr);
        response->end(dr, NULL);
    }
    int status = mariadb->dthread_connect(conn);
    if (status != MARIADB_OK) {
        response->printf(dr, "Can't connect to MariaDB server\n");
        response->cont(dr);
        response->end(dr, NULL);
    }
    mariadb->dthread_query(conn, "SET NAMES 'UTF8'");
    sprintf(use_db_query, "USE %s", db);
    mariadb->dthread_query(conn, use_db_query);
    sprintf(count_query, "SELECT COUNT(*) FROM %s", table);
    mariadb_result_t *result = mariadb->dthread_query(conn, count_query);
    assert(result);
    char **row = NULL;
    int error;
    json_t *item;
    while (1) {
        row = mariadb->dthread_get_row(conn, result, &error);
        if (!row) {
            if (error) {
                response->cont(dr);
                response->end(dr, NULL);
            } else {
                break;
            }
        }
        item = json->create_string(row[0]);
        json->add_to_array(row_nums, item);
    }
    mariadb->dthread_disconnect(conn);
    char *encoded_str = json->print_gc(dr, row_nums);
    response->printf(dr, "%s", encoded_str);
    response->cont(dr);
    response->end(dr, NULL);
}

void cb_row_nums(duda_request_t *dr)
{
    int id = dthread->create(row_nums, dr);
    dthread->resume(id);
}

void list_rows(void *data)
{
    duda_request_t *dr = data;
    assert(dr);
    json_t *row_root = json->create_object();
    char *db    = param->get(dr, 0);
    char *table = param->get(dr, 1);
    long page;
    param->get_number(dr, 2, &page);
    char use_db_query[128];
    char select_query[128];
    int page_size = 20;

    response->http_status(dr, 200);
    response->http_header_n(dr, "Content-Type: application/json", 30);
    response->wait(dr);

    mariadb_conn_t *conn = mariadb->create_conn(dr, "root", "db", "localhost",
            NULL, 0, "/run/mysqld/mysqld.sock", 0);
    if (!conn) {
        response->printf(dr, "No Connection Available\n");
        response->cont(dr);
        response->end(dr, NULL);
    }
    int status = mariadb->dthread_connect(conn);
    if (status != MARIADB_OK) {
        response->printf(dr, "Can't connect to MariaDB server\n");
        response->cont(dr);
        response->end(dr, NULL);
    }
    mariadb->dthread_query(conn, "SET NAMES 'UTF8'");
    sprintf(use_db_query, "USE %s", db);
    mariadb->dthread_query(conn, use_db_query);
    sprintf(select_query, "SELECT * FROM %s LIMIT %ld, %d", table, (page - 1) * page_size,
            page_size);

    mariadb_result_t *result = mariadb->dthread_query(conn, select_query);
    assert(result);
    json_t *field_array = json->create_array(); 
    json_t *item;
    char **fields = mariadb->dthread_get_fields(result);
    int n_fields = mariadb->dthread_get_field_num(result);
    unsigned long i;
    for (i = 0; i < n_fields; ++i) {
        item = json->create_string(fields[i]);
        json->add_to_array(field_array, item);
    }
    json->add_to_object(row_root, "fields", field_array);
    json_t *row_array = json->create_array();
    char **row = NULL;
    int error;
    while (1) {
        row = mariadb->dthread_get_row(conn, result, &error);
        if (!row) {
            if (error) {
                response->cont(dr);
                response->end(dr, NULL);
            } else {
                break;
            }
        }
        item = json->create_object();
        json_t *column;
        for (i = 0; i < n_fields; ++i) {
            if (row[i] == NULL) {
                column = json->create_null();
            } else {
                column = json->create_string(row[i]);
            }
            json->add_to_object(item, fields[i], column);
        }
        json->add_to_array(row_array, item);
    }
    json->add_to_object(row_root, "rows", row_array);
    mariadb->dthread_disconnect(conn);
    char *encoded_str = json->print_gc(dr, row_root);
    response->printf(dr, "%s", encoded_str);
    response->cont(dr);
    response->end(dr, NULL);
}

void cb_list_rows(duda_request_t *dr)
{
    int id = dthread->create(list_rows, dr);
    dthread->resume(id);
}

void cb_home_page(duda_request_t* dr)
{
    response->http_status(dr, 200);
    response->http_header_n(dr, "Content-Type: text/html", 23);
    print_header(dr);
    response->printf(dr, "\n\
  <div class=\"container\">\n\
    <div class=\"jumbotron\">\n\
      <h1>Non-blocking MariaDB Access</h1>\n\
      <p>\n\
        This is a Duda web service that demonstrates the usage of Duda MariaDB package.\n\
        If you're interested in the package itself, please refer to\n\
        <a href=\"https://github.com/swpd/duda_mariadb\">this</a>.\n\
      </p>\n\
      <p><a class=\"btn btn-primary btn-lg\" href=\"/mariadb_dthread/mariadb/dashboard/\">Get Started »</a></p>\n\
    </div>\n\
  </div>");

    char *js = "\n\
  <script>\n\
    $(document).ready(function() {\n\
      $('#home').addClass('active');\n\
    });\n\
  </script>";
    print_footer(dr, js);
    response->end(dr, NULL);
}

void cb_dashboard(duda_request_t *dr)
{
    response->http_status(dr, 200);
    response->http_header_n(dr, "content-type: text/html", 23);
    print_header(dr);
    response->printf(dr, "\n\
  <div class=\"container\">\n\
    <div class=\"panel panel-default\">\n\
      <div class=\"panel-heading\">\n\
        <h4 class=\"panel-title\">Dashboard</h4>\n\
      </div>\n\
      <div class=\"panel-body\">\n\
        <div class=\"row\">\n\
          <div class=\"col-lg-4 col-lg-offset-2\">\n\
            <form class=\"form-inline\">\n\
              <label>choose a database:</label>\n\
              <div class=\"btn-group\">\n\
                <button id=\"current-db\" type=\"button\" class=\"btn btn-default\"></button>\n\
                <button type=\"button\" class=\"btn btn-default dropdown-toggle\" data-toggle=\"dropdown\">\n\
                  <span class=\"caret\"></span>\n\
                </button>\n\
                <ul id=\"db-list\" class=\"dropdown-menu\">\n\
                </ul>\n\
              </div>\n\
            </form>\n\
          </div>\n\
          <div class=\"col-lg-4\">\n\
            <form class=\"form-inline\">\n\
              <label>choose a table:</label>\n\
              <div class=\"btn-group\">\n\
                <button id=\"current-tb\" type=\"button\" class=\"btn btn-default\"></button>\n\
                <button type=\"button\" class=\"btn btn-default dropdown-toggle\" data-toggle=\"dropdown\">\n\
                  <span class=\"caret\"></span>\n\
                </button>\n\
                <ul id=\"tb-list\" class=\"dropdown-menu\">\n\
                </ul>\n\
              </div>\n\
            </form>\n\
          </div>\n\
        </div>\n\
      </div>\n\
    </div>\n\
    <div class=\"row\">\n\
      <div class=\"col-lg-12\">\n\
        <span style=\"display: none\" id=\"total_page\"></span>\n\
        <span style=\"display: none\" id=\"current_page\"></span>\n\
        <ul id=\"pager\" class=\"pager\">\n\
        </ul>\n\
      </div>\n\
    </div>\n\
    <table id=\"rows\" class=\"table table-bordered\">\n\
    </table>\n\
  </div>");

    char *js = "\n\
  <script>\n\
    $(document).ready(function() {\n\
      $('#dashboard').addClass('active');\n\
      $.ajax({\n\
        url: '/mariadb_dthread/mariadb/list-databases/',\n\
        success: function(data) {\n\
          for (var i in data) {\n\
            $('#db-list').append('<li><a>' + data[i] + '</a></li>');\n\
          }\n\
          $('#db-list li').click(function() {\n\
            var db = $(this).text();\n\
            $('#current-db').text(db);\n\
            $.ajax({\n\
              url: '/mariadb_dthread/mariadb/list-tables/' + db + '/',\n\
              success: function(data) {\n\
                $('#tb-list').children().remove();\n\
                for (var i in data) {\n\
                  $('#tb-list').append('<li><a>' + data[i] + '</a></li>');\n\
                }\n\
                $('#tb-list li').click(function() {\n\
                  var db = $('#current-db').text();\n\
                  var tb = $(this).text();\n\
                  $('#current-tb').text(tb);\n\
                  $.ajax({\n\
                    url: '/mariadb_dthread/mariadb/row-nums/' + db + '/' + tb + '/',\n\
                    success: function(data) {\n\
                      $('#pager').children().remove();\n\
                      $('#pager').append('<li tag=\"1\"><a>&laquo;</a></li>');\n\
                      $('#pager').append('<li><a>prev</a></li>');\n\
                      var page_nums = Math.ceil(parseInt(data[0]) / 20);\n\
                      $('#total_page').text(page_nums);\n\
                      var end = page_nums <= 9 ? page_nums : 9;\n\
                      for (var i = 1; i <= end; ++i) {\n\
                        $('#pager').append('<li tag=\"' + i + '\"><a>' + i + '</a></li>');\n\
                      }\n\
                      $('#pager').append('<li><a>next</a></li>');\n\
                      $('#pager').append('<li tag=\"' + page_nums + '\"><a>&raquo;</a></li>');\n\
                      function pager_cb() {\n\
                        var selected = $(this).text();\n\
                        var total_page = parseInt($('#total_page').text());\n\
                        var page;\n\
                        if (selected === 'prev') {\n\
                          var current_page = parseInt($('#current_page').text());\n\
                          page = current_page - 1 >= 1 ? current_page - 1 : 1;\n\
                        } else if (selected === 'next'){\n\
                          var current_page = parseInt($('#current_page').text());\n\
                          page = current_page + 1 <= total_page ? current_page + 1 : total_page;\n\
                        } else {\n\
                          page = parseInt($(this).attr('tag'));\n\
                        }\n\
                        $('#current_page').text(page);\n\
                        var start = page - 4 >= 1 ? page - 4 : 1;\n\
                        var end = page + 4 <= total_page ? page + 4 : total_page\n\
                        if (start == 1) {\n\
                          end = total_page <= 9 ? total_page : 9;\n\
                        } else if (end == total_page) {\n\
                          start = total_page > 9 ? total_page - 8 : 1;\n\
                        }\n\
                        $('#pager').children().remove();\n\
                        $('#pager').append('<li tag=\"1\"><a>&laquo;</a></li>');\n\
                        $('#pager').append('<li><a>prev</a></li>');\n\
                        for (var i = start; i <= end; ++i) {\n\
                          $('#pager').append('<li tag=\"' + i + '\"><a>' + i + '</a></li>');\n\
                        }\n\
                        $('#pager').append('<li><a>next</a></li>');\n\
                        $('#pager').append('<li tag=\"' + total_page + '\"><a>&raquo;</a></li>');\n\
                        $('#pager li[tag=\"' + page + '\"]').addClass('disabled');\n\
                        $('#pager li').click(pager_cb);\n\
                        $.ajax({\n\
                          url: '/mariadb_dthread/mariadb/list-rows/' + db + '/' + tb + '/' + page + '/',\n\
                          success: function(data) {\n\
                            $('#rows').children().remove();\n\
                            $('#rows').append('<tr>');\n\
                            for (var i in data.fields) {\n\
                              $('#rows tr').last().append('<td>' + data.fields[i] + '</td>');\n\
                            }\n\
                            $('#rows').append('</tr>');\n\
                            for (var ri in data.rows) {\n\
                              $('#rows').append('<tr>');\n\
                              for (var fi in data.fields) {\n\
                                $('#rows tr').last().append('<td>' + data.rows[ri][data.fields[fi]] + '</td>');\n\
                              }\n\
                              $('#rows').append('</tr>');\n\
                            }\n\
                          }\n\
                        });\n\
                      }\n\
                      $('#pager li').click(pager_cb);\n\
                      $('#pager li').first().click();\n\
                    }\n\
                  });\n\
                });\n\
                $('#tb-list li').first().click();\n\
              }\n\
            });\n\
          });\n\
          $('#db-list li').first().click();\n\
        }\n\
      });\n\
    });\n\
  </script>";
    print_footer(dr, js);
    response->end(dr, NULL);
}

int duda_main()
{
    duda_interface_t *if_system;
    duda_method_t *method;
    duda_param_t *params;

    duda_load_package(mariadb, "mariadb");
    duda_load_package(json, "json");

    if_system = map->interface_new("mariadb");

    method = map->method_new("home", "cb_home_page", 0);
    map->interface_add_method(method, if_system);

    method = map->method_new("dashboard", "cb_dashboard", 0);
    map->interface_add_method(method, if_system);

    method = map->method_new("list-databases", "cb_list_databases", 0);
    map->interface_add_method(method, if_system);

    method = map->method_new("list-tables", "cb_list_tables", 1);
    params = map->param_new("database", 64);
    map->method_add_param(params, method);
    map->interface_add_method(method, if_system);

    method = map->method_new("row-nums", "cb_row_nums", 2);
    params = map->param_new("database", 64);
    map->method_add_param(params, method);
    params = map->param_new("table", 64);
    map->method_add_param(params, method);
    map->interface_add_method(method, if_system);

    method = map->method_new("list-rows", "cb_list_rows", 3);
    params = map->param_new("database", 64);
    map->method_add_param(params, method);
    params = map->param_new("table", 64);
    map->method_add_param(params, method);
    params = map->param_new("page", 8);
    map->method_add_param(params, method);
    map->interface_add_method(method, if_system);

    duda_service_add_interface(if_system);
    return 0;
}
