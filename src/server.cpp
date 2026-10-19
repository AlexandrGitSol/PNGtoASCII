#include <asio.hpp>
#include <iostream>
using namespace std;

int main(){

    asio::io_context ioc;
    asio::ip::tcp::acceptor acceptor(ioc, asio::ip::tcp::endpoint(asio::ip::make_address("0.0.0.0"), 8080));

    asio::ip::tcp::socket sock(ioc);
    acceptor.accept(sock);

    string bin = "";
    char input[1024];

    while(true){

        size_t length = sock.read_some(asio::buffer(input));
        for (int i = 0; i < length; ++i){
            bin += input[i];
        }
        //дальше нужно парсить и искать в bin \r\n\r\n - это конец первого запроса
        string request = "", behind_request = "";
        bool isrequest = true;
        for (int i = 0; i < bin.length()-4; ++i){
            if (isrequest){
                request += bin[i];
            }
            else{
                behind_request += bin[i];
            }
            if (bin.substr(i, 4) == "\r\n\r\n"){
                isrequest = false;
            }
        }
        
        if (!isrequest){
            behind_request += bin.substr(bin.length()-4, 4);
            bin = behind_request;
        }

        //Парсим длинну контента
        string content_l = "";
        for (int i = 0; i < request.length()-16; ++i){
            if (request.substr(i, 16) == "Content-Length: "){
                while(isdigit(request[i + 16])){
                    content_l += request[i+16];
                    ++i;
                }
                //Указатель в конец строки
                i = request.length() - 16;
            }
        }

        //Парсим контент
        string behind_content = "";
        string content = "";
        for (int i = 0; i < bin.length(); ++i){
            if (i < stoi(content_l)){
                content += bin[i];
            }
            else{
                behind_content += bin[i];
            }
        }
        bin = behind_content;
        
        /*
        Имеем 
        std::cout << request << endl;
        std::cout << content_l << endl;
        std::cout << content << endl;
        */

        
        std::cout << bin << endl;
    }
    return 0;
}