#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h> //��������ַ��
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h> //toupper ��Сдת��Ϊ��д��
#include <sys/ioctl.h>
#include <cstdio>
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
#include <sys/stat.h>
#include <sstream>
#include <mysql.h> // mysql����
#include <sys/select.h>
using namespace std;
#define port 20521
#define _SHOW_HTTP_REQUEST 1
#define _SHOW_POST_BODY 1
#define _SHOW_SIGNIN_INF 1
#define _SHOW_SIGNUP_INF 1
#define _LOG_COL_LENGTH 80
#define _SIGN_IN_AWAIT 0
const char *host_name = "120.55.41.240";

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
    myDBConf->_password = "Tgx226057602.";
    myDBConf->_database = "assign";
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
int send_to_http_response(int client_socket, string status)
{
    string response = "HTTP/1.1 200 OK\r\n"
                      "Access-Control-Allow-Origin:http://localhost:3000\r\n"
                      "Access-Control-Allow-Methods: POST, GET, OPTIONS \r\n"
                      "Access-Control-Allow-Headers: x-requested-with,X-PINGOTHER, Content-Typ\r\n"
                      "Access-Control-Allow-Credentials: true\r\n"
                      "Content-type: application/x-www-form-urlencoded; charset=GBK2312\r\n"
                      "\r\n";
    //"{\"status\":\"OK\"\r\n}";
    // char not_found_response_template[1024] =
    //     "HTTP/1.1 200 OK\r\n"
    //     "Access-Control-Allow-Origin: http://localhost:3000\r\n"
    //     "Content-type: application/x-www-form-urlencoded; charset=GBK2312\r\n"
    //     "\r\n";
    response += status;
    // strcat(not_found_response_template, status);
    send(client_socket, response.c_str(), response.length(), 0);
    // int req = strlen(not_found_response_template);
    // send(client_socket, not_found_response_template, req, 0);
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
    char request[1024 * 1024];
    int req = recv(client_socket, request, sizeof(request), 0);
    // while (1)
    // {
    //     req += recv(client_socket, request, sizeof(request), 0);
    //     cout << req << endl;
    //     if (req >= 100000||req==460||req==507)
    //         break;
    // }
    cout << req << endl;
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

int no_block(int fd)
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
    no_block(server_socket);
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

void make_response(string &response, string item, string value)
{
    response = response.substr(0, response.length() - 1);
    cout << response << endl;
    if (response.length() != 1)
    {
        response += ",";
    }
    response = response + "\"" + item + "\":\"" + value + "\"}";
}
void http_signin(MyDB *mydatabase, string &status, message *mes)
{
    string username, password;
    print_log_top(_SHOW_SIGNIN_INF, "�����¼");
    get_value(mes, "username", username);
    get_value(mes, "password", password);
    if (mydatabase->IsPwdCorrent(username, password))
        make_response(status, "status", "OK");
    else
        make_response(status, "status", "WRONG");
    // strcpy(status, "OK");
    print_log_bottum(_SHOW_SIGNIN_INF);
}
void http_signup(MyDB *mydatabase, string &status, message *mes)
{
    string username, password;
    print_log_top(_SHOW_SIGNUP_INF, "����ע��");
    get_value(mes, "username", username);
    get_value(mes, "password", password);
    if (mydatabase->InsertNewUser(username, password))
    {
        make_response(status, "status", "OK");
        write_log_sign_up(mes);
    }
    else
        make_response(status, "status", "WRONG");
    print_log_bottum(_SHOW_SIGNUP_INF);
}

void read_from_http_to_file(int client_socket, string file_name)
{
    no_block(client_socket);
    char request[1024];
    int fd = open(file_name.c_str(), O_WRONLY | O_CREAT);
    if (fd < 0)
    {
        perror("open file while writing");
        exit(1);
    }
    fd_set rest;
    /*select��ʱ����*/
    struct timeval tempval;  //select�ȴ�ʱ��
    tempval.tv_sec = 0;      //select�ȴ�����
    tempval.tv_usec = 50000; //select�ȴ�΢����
    int total = 0;
    int req = 0;
    while (1)
    {
        total = 0;
        FD_ZERO(&rest);                                                             //��ն�������
        FD_SET(client_socket, &rest);                                               //�Ѽ����׽��ַ����������
        int select_return = select(client_socket + 1, &rest, NULL, NULL, &tempval); //�����׽ӵĿɶ��Ϳ�д����
        if (select_return == 0)
            break;
        if (select_return < 0)
        {
            printf("select error\n");
            exit(1);
        }
        if (FD_ISSET(client_socket, &rest))
        {
            req = read(client_socket, request, sizeof(request));

            while (1)
            {
                if (total == req)
                    break;
                int weq = write(fd, &request[total], req);
                total += weq;
            }
        }
    }
    close(fd);
}
int read_line(int fd, string &one_line)
{
    char one_letter;
    one_line = "";
    while (1)
    {
        int red = read(fd, &one_letter, 1);
        if (red == -1)
        {
            perror("read file");
            close(fd);
            return -1;
        }
        if (red == 0)
            break;
        one_line += one_letter;

        if (one_letter == '\n')
            break;
    }
    return 0;
}
int analysis_request_message(string file_name, string &boundary_or_request, message *mes)
{

    int fd = open(file_name.c_str(), O_RDONLY);
    string one_line;
    while (1)
    {
        if (read_line(fd, one_line) == -1)
        {
            return -1; //����-1�������
        }
        if (one_line[0] == '{')
        {
            boundary_or_request = one_line;
            int i = 0;
            int m = 0;
            while (1)
            {
                i += get_in_syh(&one_line[++i], mes[m].item);
                i += get_in_syh(&one_line[++i], mes[m].value);
                m++;
                if (one_line[i] == '}')
                    break;
            }

            close(fd);
            return 0; //����0 ����Ϊ���ļ�����
        }
        int b_locate = one_line.find("boundary");
        if (b_locate != -1)
        {
            boundary_or_request = one_line.substr(b_locate + 9, one_line.length() - b_locate - 11);
            return fd;
        }
    }
}
int ayalysis_file_message(int fd, string &boundary_or_request, message *mes)
{
    string one_line;
    string boundary = boundary_or_request;
    int n_boundary = 0;
    boundary = "--" + boundary;
    while (1)
    {

        if (read_line(fd, one_line) == -1)
        {
            return -1; //����-1�������
        }
        one_line = one_line.substr(0, one_line.length() - 2);
        if (one_line == boundary)
        {

            n_boundary++;
            while (1)
            {
                if (read_line(fd, one_line) == -1)
                {
                    return -1; //����-1�������
                }
                if (one_line == "\r\n")
                    break;
            }

            if (n_boundary == 1)
            {
                if (read_line(fd, one_line) == -1)
                {
                    return -1; //����-1�������
                }
                boundary_or_request = one_line;
                int i = 0;
                int m = 0;
                while (1)
                {
                    i += get_in_syh(&one_line[++i], mes[m].item);
                    i += get_in_syh(&one_line[++i], mes[m].value);
                    m++;
                    if (one_line[i] == '}')
                        break;
                }
            }
            //cout << n_boundary << endl;

            if (n_boundary == 2)
            {
                string old_line;
                string path;
                string chunk;
                get_value(mes, "chunk", chunk);
                string hash;
                get_value(mes, "hash", hash);
                path = "/home/tempfile/" + hash + "/" + chunk;
                cout << path << endl;
                int new_fd = open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND);
                if (new_fd < 0)
                {
                    perror("open new file");
                    return -1; //ʧ�ܷ���-1
                }
                one_line = "";
                int o = 0;

                while (1)
                {
                    old_line = one_line;
                    if (read_line(fd, one_line) == -1)
                    {
                        return -1; //����-1�������
                    }
                    // cout << one_line;
                    // cout << (boundary + "--\r\n");
                    // cout << (one_line == boundary + "--");
                    // getchar();

                    if (one_line == boundary + "--\r\n")
                    {
                        close(fd);
                        o = 2;
                    }
                    if (old_line != "")
                    {
                        int weq = write(new_fd, old_line.c_str(), old_line.length() - o);
                        //cout << weq << endl;
                    }
                    if (o == 2)
                    {
                        close(new_fd);
                        return 1;
                    }
                }
            }
        }
    }
}

void file_upload(MyDB *mydatabase, string &status, message *mes)
{
    cout << "dd" << endl;
    //�ж��
    string hash;
    get_value(mes, "hash", hash);
    hash = "/home/tempfile/" + hash;
    cout << hash << endl;
    if (access(hash.c_str(), 0) == -1) //�������ļ���
                                       //�½��ļ���
        mkdir(hash.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

int main(int argc, char **argv)
{

    if (access("/home/temp", 0) == -1) //�������ļ���
                                       //�½��ļ���
        mkdir("/home/temp", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (access("/home/tempfile", 0) == -1) //�������ļ���
                                           //�½��ļ���
        mkdir("/home/tempfile", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    // int msgid;
    // int reval;
    // int sflags;
    // send_queue *mysend_queue = new send_queue[100];
    // queue<int> usingnum_of_queue;  //��ǰ����ʹ�õ��ϴ��ļ��ṹ�����;
    // queue<int> available_of_queue; //���õ��ϴ��ļ��Ľṹ�����
    // msgid = CreateMsgQueue();

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
                string boundary_or_request;
                int client_socket = http_new_connection(server_socket, pin);

                stringstream ss;
                ss << "/home/temp/tempmessage_" << getpid();
                string filename;
                filename = ss.str();
                read_from_http_to_file(client_socket, filename);
                int fd = analysis_request_message(filename, boundary_or_request, mes);
                cout << fd << endl;
                string process; //��ǰhttp����������
                string response = "{}";
                cout << boundary_or_request << endl;
                char status[10] = "WRONG"; //http��Ӧ����������ʼΪ����
                if (fd == 0)
                {
                    get_value(mes, "process", process); //��ȡ��������
                    cout << process << endl;
                    if (process == "signin")
                        http_signin(mydatabase, response, mes);
                    else if (process == "signup")
                        http_signup(mydatabase, response, mes);
                    else if (process == "uploadRequest")
                        file_upload(mydatabase, response, mes);
                    //��boundary_or_request����ֱ��д����Ϣ���У�
                }
                if (fd != 0)
                {
                    ayalysis_file_message(fd, boundary_or_request, mes);
                }
                // read_from_http_request(client_socket, mes); //������ҳǰ�˵�http����
                send_to_http_response(client_socket, response); //����ҳǰ�˷���http��Ӧ
                http_close_connection(client_socket, pin);
                if (remove(filename.c_str()) != 0)
                    cout << "ɾ��ʧ��\n";
                return (EXIT_SUCCESS);
            }
        }
    }
    close(server_socket);
    return (EXIT_SUCCESS);
}
