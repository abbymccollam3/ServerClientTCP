/*
Author: Abby McCollam
Class: ECE4122 Section A
Last Date Modified: 11/26/23
Description: Client communicating to server
*/

//headers
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <omp.h>
#include <algorithm>
#include <cstdlib>

//creating instances of classes
sf::Packet packet;
sf::TcpSocket socket;
sf::SocketSelector selector;

using namespace std;

struct tcpMessage //Data using following packet structure
{
    unsigned char nVersion;
    unsigned char nType;
    unsigned short nMsgLen;
    char chMsg[1000];
};

//initializing message structure
tcpMessage rcvMessage = {102, 77, 1, ' '};

void ifTypeV(unsigned long var) //if v is entered
{
    unsigned char version;
    if (var <= 255)
    {
        version = static_cast<unsigned char>(var); //converts to char
        rcvMessage.nVersion = version; //changes version
    }
    else
        cout << "Please enter valid input." << endl;
}

void ifTypeT(string mes, unsigned long var) //if t is entered
{
    unsigned char newType;

    if (var <= 255)
    {
        newType = static_cast<unsigned char>(var); //converts to char
        rcvMessage.nType = newType; //sets nType
        mes = mes.erase(0, 1);
        strcpy(rcvMessage.chMsg, mes.c_str()); //copies content of messages

        packet << rcvMessage.nVersion << rcvMessage.nType << rcvMessage.nMsgLen << rcvMessage.chMsg; //forms packet

        if (socket.send(packet) != sf::Socket::Done)
        {
            //continue;
        }
    }
    else
        cout << "Please enter valid input." << endl;
}

void ifTypeQ()
{
    rcvMessage.nType = 1; //changing version to 1
    packet << rcvMessage.nVersion << rcvMessage.nType >> rcvMessage.nMsgLen << rcvMessage.chMsg; //sending packet to close connection
    if (socket.send(packet) != sf::Socket::Done)
    {
        //continue;
    }
    cout << "Socket closed." << endl;
    exit(1);
}

int main(int argc, char* argv[])
{
    if (argc != 3) //checks for valid number of input arguments
    {
        cout << "Usage:./ClientTCP <IP Address> <port>" << endl;
        return 1;
    }

    sf::IpAddress serverIP(argv[1]); //initialize IP address
    const auto port = static_cast<unsigned short>(stoi(argv[2]));

    if (socket.connect(serverIP, port) != sf::Socket::Done) //connects socket
    {
        return 0;
    }

    selector.add(socket); //adding to selector to monitor activity

    //BEGIN OPENMP
#pragma omp parallel sections
    {
#pragma omp section
        {
            while (true)
            {
                unsigned long temp;
                string command, foo, index, message;

                cout << "Please enter command: ";
                getline(cin, command);

                if (command[1] != ' ') //if not valid input
                {
                    cout << "Please enter valid input." << endl;
                    continue;
                }

                index = command.substr(0, 1);
                command.erase(0,2);

                if (command.size() == 0)
                {
                    temp = 0;
                }
                else //deciphering input commands from input line
                {
                    size_t spacePos = command.find(' ');
                    foo = command.substr(0, spacePos);
                    temp = stoul(foo, nullptr, 0);
                    command.erase(0, spacePos);
                }

                if (command.size() != 0)
                {
                    message = command;
                }

                if (index == "v") //if v command
                {
                    ifTypeV(temp);
                }
                else if (index == "t") //if t command
                {
                    ifTypeT(message, temp);
                }
                else if (index == "q") //if q command, terminate program
                {
                    ifTypeQ();
                }
                cin.clear();
            }
        }
#pragma omp section
        {
            while (true)
            {
                if (selector.wait(sf::milliseconds(10.f))) //checking for activity on sockets
                {
                    if (selector.isReady(socket)) //checks if socket is ready to receive data
                    {
                        if (socket.receive(packet) != sf::Socket::Done) //receiving packets
                        {
                            continue;
                        }

                        packet >> rcvMessage.nVersion >> rcvMessage.nType >> rcvMessage.nMsgLen >> rcvMessage.chMsg; //extracts received message
                        if (rcvMessage.nVersion == 1) //if packet has nVersion = 1, close connection
                        {
                            cout << "Connection closed from server." << endl;
                            exit(1);
                        }
                        else //otherwise output this information
                        {
                            cout << "Received Msg Type: " << +rcvMessage.nType << "; Msg: " << rcvMessage.chMsg << endl;
                            continue;
                        }
                    }
                }
            }
        }
    }
    cout << "Goodbye." << endl;
    return 0;
}