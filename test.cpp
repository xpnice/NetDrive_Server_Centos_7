#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h> //��������ַ��
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h> //toupper ��Сдת��Ϊ��д��
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <iomanip>
#include <mysql.h> // mysql����
using namespace std;
#define port 20521
#define _SHOW_HTTP_REQUEST 1
#define _SHOW_POST_BODY 1
#define _SHOW_SIGNIN_INF 1
#define _SHOW_SIGNUP_INF 1
#define _LOG_COL_LENGTH 80
#define _SIGN_IN_AWAIT 0
const char *host_name = "192.168.193.80";

typedef struct message
{
    string item;
    string value;

} message; //��json�ṹ��

typedef enum //���ݿ����ö����
{
    INSERT,
    DELETE,
    SELECT,
    UPDATE
} exec;

struct DBConf //���ݿ��������
{
    string _host;     //������ַ
    string _user;     //�û���
    string _password; //����
    string _database; //���ݿ�
    string _charset;  //�ַ���
    int _port;        //�˿�
    int _flag;        //�ͻ��˱�ʶ
};

//���ݿ��ʼ��
void initDBConf(DBConf *myDBConf)
{
    myDBConf->_host = "localhost";
    myDBConf->_user = "root";
    myDBConf->_password = "Tielin0303!";
    myDBConf->_database = "team3";
    myDBConf->_charset = "gbk";
    myDBConf->_port = 0;
    myDBConf->_flag = 0;
}

class MyDB
{
public:
    MyDB();
    ~MyDB();
    bool InitDB(DBConf myDBConf);
    bool ExeSQL(string sql, exec oper);
    bool IsUserExist(string username);
    void PrintResult();
    bool InsertNewUser(string username, string pwd);
    void get_my_time(string &ctime);
    bool IsResultEmpty();
    bool IsPwdCorrent(string username, string pwd);

private:
    MYSQL *mysql;
    MYSQL_ROW row;
    MYSQL_RES *result;
    MYSQL_FIELD *field;
};

MyDB::MyDB()
{
    mysql = mysql_init(NULL);
    if (mysql == NULL)
    {
        cout << "mysql_init failed" << endl;
        exit(-1);
    }
}

MyDB::~MyDB()
{
    if (!mysql)
    {
        mysql_close(mysql);
    }
}

bool MyDB::InitDB(DBConf myDBConf)
{
    /*�������ݿ�*/
    if (mysql_real_connect(mysql, myDBConf._host.c_str(), myDBConf._user.c_str(), myDBConf._password.c_str(), myDBConf._database.c_str(), myDBConf._port, NULL, myDBConf._flag) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        exit(-1);
    }
    /* �����ַ���������������ַ����룬��ʹ/etc/my.cnf������Ҳ���� */
    mysql_set_character_set(mysql, myDBConf._charset.c_str());
    return true;
}

/* ���в�ѯ���ɹ�����0�����ɹ���0
  1����ѯ�ַ��������﷨����
  2����ѯ�����ڵ����ݱ� */
bool MyDB::ExeSQL(string sql, exec oper)
{
    /*ִ��ʧ��*/
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        exit(1);
    }
    else if (oper == SELECT)
    {
        /* ����ѯ����洢���������ִ����򷵻�NULL
       ע�⣺��ѯ���ΪNULL�����᷵��NULL */
        result = mysql_store_result(mysql);
    }
    return true;
}

void MyDB::PrintResult()
{
    if (result != NULL)
    {
        int fieldnum = mysql_num_fields(result);
        for (int i = 0; i < fieldnum; i++)
        {
            row = mysql_fetch_row(result);
            if (row <= 0)
                break;
            for (int j = 0; j < fieldnum; j++)
            {
                cout << row[j] << "\t\t";
            }
            cout << endl;
        }
        mysql_free_result(result);
    }
    else
        cout << "mysql_store_result failed" << endl;
}

bool MyDB::IsResultEmpty()
{
    if (result != NULL)
    {
        int rownum = mysql_num_rows(result);
        if (rownum != 0)
        {
            mysql_free_result(result);
            return false;
        }
        mysql_free_result(result);
        return true;
    }
}

//��ѯ�û��Ƿ������ݿ��д��ڣ������ڣ��ж��������Ƿ���ȷ
//�����û�����
bool MyDB::IsUserExist(string username)
{
    exec oper = SELECT;
    string sqlsen = "select * from user ";
    sqlsen += "where username = \"" + username + "\"";
    ExeSQL(sqlsen, oper);
    bool flag = IsResultEmpty();
    if (flag == false) //��������Ϊ�գ����û�����
    {
        if (_SHOW_SIGNUP_INF || _SHOW_SIGNIN_INF)
            cout << "�û�" << username << "����" << endl;
        return true;
    }
    if (_SHOW_SIGNUP_INF || _SHOW_SIGNIN_INF)
        cout << "�û�" << username << "������" << endl;
    return false;
}

//�ж�user�û��������Ƿ���ȷ
bool MyDB::IsPwdCorrent(string username, string pwd)
{
    //���ж��û��Ƿ����
    bool flag = IsUserExist(username);
    if (flag == false)
    {
        return false;
    }
    //���û�����
    exec oper = SELECT;
    string sqlsen = "select * from user ";
    sqlsen += "where username = \"" + username + "\" and password = \"" + pwd + "\"";
    ExeSQL(sqlsen, oper);
    flag = IsResultEmpty();
    if (flag == true) //���Ϊ�գ������������
    {
        if (_SHOW_SIGNIN_INF)
            cout << "�������" << endl;
        return false;
    }
    if (_SHOW_SIGNIN_INF)
        cout << "������ȷ" << endl;
    return true;
}

void MyDB::get_my_time(string &ctime)
{
    time_t timep;
    struct tm *p;
    time(&timep);
    p = gmtime(&timep);
    char s[30];
    sprintf(s, "%4d-%02d-%02d", (1900 + p->tm_year), (1 + p->tm_mon), (1 + p->tm_mday));
    ctime = string(s);
}

bool MyDB::InsertNewUser(string username, string pwd)
{
    //�ڲ����û�ǰ����ѯ���û��Ƿ����
    bool flag = IsUserExist(username);
    if (flag == true)
        return false;
    exec oper = INSERT;
    //��ȡ��ǰʱ�䣬��-��-��
    string ctime;
    get_my_time(ctime);
    string sqlsen = "insert into user(username,password,create_time,valume,used) values";
    sqlsen += "( \"" + username + "\",\"" + pwd + "\",\"" + ctime + "\",2000,0)";
    ExeSQL(sqlsen, oper);
    if (_SHOW_SIGNUP_INF)
        cout << "�ѽ����û���ӵ����ݿ���" << endl;
    return true;
}

int get_in_syh(string buffer, string &answer)
{
    int b = 0;
    //cout << "wodaole\n";
    if (buffer[b] == '"')
    {
        while (1)
        {
            if (buffer[++b] == '"')
            {
                return b + 1;
            }
            answer += buffer[b];
        }
    }
}
//1�� 0��
int get_value(message *mes, string item, string &value)
{
    int i = 0;
    for (i = 0; i < 10; i++)
    {
        if (mes[i].item == item)
        {
            value = mes[i].value;
            return 1;
        }
    }
    if (i == 10)
        return -1;
}
int send_to_http_response(int client_socket, char *status)
{
    char not_found_response_template[1024] =
        "HTTP/1.1 200 OK\r\n"
        "Access-Control-Allow-Origin: http://localhost:3000\r\n"
        "Content-type: application/x-www-form-urlencoded; charset=GBK2312\r\n"
        "\r\n"
        "{\"status\":\"";
    strcat(not_found_response_template, status);
    strcat(not_found_response_template, "\"}");
    int req = strlen(not_found_response_template);
    send(client_socket, not_found_response_template, req, 0);
    return 1;
}
void print_log_top(int condition, const char *str)
{
    if (condition)
    {
        ios init(NULL);
        init.copyfmt(cout);
        cout << setfill('-') << setw(_LOG_COL_LENGTH / 2) << str << setfill('-') << setw(_LOG_COL_LENGTH / 2) << '-' << endl;
        cout.copyfmt(init);
    }
}
void print_log_bottum(int condition)
{
    if (condition)
    {
        ios init(NULL);
        init.copyfmt(cout);
        cout << setfill('-') << setw(_LOG_COL_LENGTH) << '-' << endl;
        cout.copyfmt(init);
    }
}
void read_from_http_request(int client_socket, message *mes)
{
    char request[1024];
    int req = recv(client_socket, request, sizeof(request), 0);
    request[req] = '\0';
    string buffer = (string)request;
    if (req == -1)
    {
        perror("read");
        exit(1);
    }
    int i = 0;
    int m = 0;
    if (_SHOW_HTTP_REQUEST)
    {
        print_log_top(_SHOW_HTTP_REQUEST, "������");
        cout << buffer << endl;
        print_log_bottum(_SHOW_HTTP_REQUEST);
    }
    while (1)
    {
        if (buffer[i] == '{')
        {
            print_log_top(_SHOW_POST_BODY, "BODY����");
            while (1)
            {
                i += get_in_syh(&buffer[++i], mes[m].item);
                i += get_in_syh(&buffer[++i], mes[m].value);
                if (_SHOW_POST_BODY)
                {
                    ios init(NULL);
                    init.copyfmt(cout);
                    cout << std::left << setw(15) << mes[m].item << " : ";
                    cout << std::left << setw(49) << mes[m].value << endl;
                    cout.copyfmt(init);
                }
                m++;
                if (buffer[i] == '}')
                    break;
            }
            if (buffer[i] == '}')
                break;
        }
        i++;
    }
    print_log_bottum(_SHOW_POST_BODY);
}

int no_block(int fd, const char *str)
{

    int flags;
    if ((flags = fcntl(fd, F_GETFL, NULL)) < 0)
    {

        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {

        return -1;
    }
    //printf("%s no block set successful\n", str);
}
int http_new_connection(int server_socket, struct sockaddr_in pin)
{
    socklen_t address_size;
    int client_socket = accept(server_socket, (struct sockaddr *)&pin, &address_size);
    if (client_socket == -1)
        exit(1);
    ios init(NULL);
    init.copyfmt(cout);
    cout << endl
         << endl
         << setfill('*') << setw(_LOG_COL_LENGTH) << '*' << endl;
    printf("���ӵ��ͻ��� CLIENT_IP:%s CLIENT_PORT:%d\n",
           inet_ntoa(pin.sin_addr),
           ntohs(pin.sin_port));

    cout.copyfmt(init);
    return client_socket;
}
void error_exit(const char *str)
{
    perror(str);
    exit(1);
}
int http_close_connection(int client_socket, struct sockaddr_in pin)
{
    usleep(_SIGN_IN_AWAIT);
    close(client_socket);
    ios init(NULL);
    init.copyfmt(cout);
    printf("�ͻ��� CLIENT_IP:%s CLIENT_PORT:%d �Ͽ�����\n",
           inet_ntoa(pin.sin_addr),
           ntohs(pin.sin_port));
    cout << setfill('*') << setw(_LOG_COL_LENGTH) << '*' << endl;
    cout.copyfmt(init);
}
/*�����*/
void write_log_sign_up(message *mes)
{

    string a = "{\n\t\"username\": \"";
    string b = "\",\n\t\"content\": [\n\t]\n}";
    string username_value;
    get_value(mes, "username", username_value);
    string dir_name = "mkdir /home/";
    dir_name += username_value;

    system(dir_name.c_str());
    string file_name = &dir_name[6] + string("/message.log");
    int fd = open(file_name.c_str(), O_WRONLY | O_CREAT);
    if (fd == -1)
    {
        perror("open");
        exit(1);
    }
    write(fd, a.c_str(), a.length());
    write(fd, username_value.c_str(), username_value.length());
    write(fd, b.c_str(), b.length());
    close(fd);
}
int listen_socket_init()
{
    struct sockaddr_in sin; //struct sockaddr��struct sockaddr_in�������ṹ��������������ͨ�ŵĵ�ַ��
                            //  �׽ӿ�������
    //socket��������ϵͳ����һ��ͨ�Ŷ˿�
    int server_socket = socket(AF_INET, SOCK_STREAM, 0); //IPV4 TCPЭ��
    if (server_socket == -1)                             //����ʧ��
        error_exit("call to socket");
    //�˿��ظ�����
    int one = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0)
    {
        close(server_socket);
        exit(1);
    }
    no_block(server_socket, "SOCKET");
    bzero(&sin, sizeof(sin));         // ��ʼ�� Ȼ���������׽���
    sin.sin_family = AF_INET;         //Э���壬��socket�����ֻ����AF_INET(TCP/IPЭ����)
    sin.sin_addr.s_addr = INADDR_ANY; //sin_addr�洢IP��ַ,ʹ��in_addr������ݽṹ
                                      //s_addr���������ֽ�˳��洢IP��ַ
                                      //in_addr32λ��IPv4��ַ
    sin.sin_port = htons(port);       //�洢�˿ں�
    //no_block(server_socket);
    if (bind(server_socket, (struct sockaddr *)&sin, sizeof(sin)) == -1)
        error_exit("call to bind");
    if (listen(server_socket, 200) == -1) //�ڶ˿�server_socket����
        error_exit("call to listen");
    return server_socket;
}
void fd_set_init(fd_set *rest, fd_set *west, int server_socket)
{
    struct timeval tempval;
    tempval.tv_sec = 0;
    tempval.tv_usec = 0;
    FD_ZERO(rest);
    FD_ZERO(west);
    FD_SET(server_socket, rest);
    FD_SET(server_socket, west);
    int flag = select(server_socket + 1, rest, west, NULL, &tempval); //�����׽ӵĿɶ��Ϳ�д����
    if (flag < 0)
        error_exit("select error");
}
void http_signin(MyDB *mydatabase, char *status, message *mes)
{
    string username, password;
    print_log_top(_SHOW_SIGNIN_INF, "�����¼");
    get_value(mes, "username", username);
    get_value(mes, "password", password);
    if (mydatabase->IsPwdCorrent(username, password))
        strcpy(status, "OK");
    print_log_bottum(_SHOW_SIGNIN_INF);
}
void http_signup(MyDB *mydatabase, char *status, message *mes)
{
    string username, password;
    print_log_top(_SHOW_SIGNUP_INF, "����ע��");
    get_value(mes, "username", username);
    get_value(mes, "password", password);
    if (mydatabase->InsertNewUser(username, password))
    {
        strcpy(status, "OK");
        write_log_sign_up(mes);
    }
    print_log_bottum(_SHOW_SIGNUP_INF);
}
int main(int argc, char **argv)
{
    //���ݿ��ʼ��
    MyDB *mydatabase = new MyDB();
    DBConf myDBConf;
    initDBConf(&myDBConf);
    mydatabase->InitDB(myDBConf);
    //�����׽��ֳ�ʼ��
    int server_socket = listen_socket_init();
    //�����ַ���
    fd_set rest, west;
    //����αjson�ṹ������
    message *mes = new message[10];
    while (1)
    {
        fd_set_init(&rest, &west, server_socket);
        if (FD_ISSET(server_socket, &rest))
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                struct sockaddr_in pin;
                int client_socket = http_new_connection(server_socket, pin);
                read_from_http_request(client_socket, mes); //������ҳǰ�˵�http����
                string process;                             //��ǰhttp����������
                char status[10] = "WRONG";                  //http��Ӧ����������ʼΪ����
                get_value(mes, "process", process);         //��ȡ��������
                if (process == "signin")
                    http_signin(mydatabase, status, mes);
                else if (process == "signup")
                    http_signup(mydatabase, status, mes);
                send_to_http_response(client_socket, status); //����ҳǰ�˷���http��Ӧ
                http_close_connection(client_socket, pin);
                return (EXIT_SUCCESS);
            }
        }
    }
    close(server_socket);
    return (EXIT_SUCCESS);
}
