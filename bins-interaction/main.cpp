#include <QCoreApplication>
#include <QSerialPort>
#include <iostream>
#include <fstream>
#include <QDebug>
#include <QByteArray>
#include <cstring>
#include <iomanip>
#include <QTimer>
#include <unistd.h>
#include <QHash>
#include <bitset>
#include <cstddef>
#include <cstring>
#include <QFile>
#include <QTime>


float heading = 0.0;
float roll = 0.0;
float pitch = 0.0;
double latitude = 0.0;
double longitude = 0.0;
float longitudeAccuracy = 0.0;
float latitudeAccuracy = 0.0;
float pitchAccuracy = 0.0;
float rollAccuracy = 0.0;
float headingAccuracy = 0.0;
float EAST_VELOCITY = 0.0;
float NORTH_VELOCITY = 0.0;
float VERTICAL_VELOCITY = 0.0;
float EAST_VELOCITY_Accuracy = 0.0;
float NORTH_VELOCITY_Accuracy = 0.0;
float VERTICAL_VELOCITY_Accuracy = 0.0;
int STATUS = 0.0;

// ******************************************************
// Взаимодействие ПК - БИНС H1

// CRC в соответствии с алгоритмом БИНС
unsigned char crc(void const* mem, size_t len)
{
    unsigned char const* data = static_cast<unsigned char const*>(mem);
    uint8_t crc = 0xFF;
    while(len--)
    {
        crc^= *data++;
        for(int i = 0; i < 8; i++)
        {
            crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
            //qDebug() << "crc function = " << crc;
        }
    }
    return crc;
}
// Передача сообщения БИНС через последовательный порт по таймеру
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //Параметры COM в соответствии с документацией БИНС
    QSerialPort port;
    port.setPortName("COM1");
    qint32 baudRate = 921600; //921600
    port.setBaudRate(baudRate);
    port.setStopBits(QSerialPort::StopBits(1));
    port.setDataBits(QSerialPort::DataBits(8));
    port.setParity((QSerialPort::NoParity));
    if(!port.open((QIODevice::ReadWrite)))
        qDebug() << "error no open COM port";
    else qInfo() << "bins_console, open COM port: ok, baudRate:" <<baudRate;

    // Передаваемое сообщение
    /* {0xFB, 0x01, 0x00, 0x01};*/      //Addr reg: 0x00
    /*{0xFB, 0xFF, 0x01, 0x01}*/        //Addr reg: 0x01 CRC=0x80
    //{0xFB, 0x01, 0x04, 0x81, 0x00, 0x00, 0x00, 0x03} на запись
    //{0xFB, 0x01, 0x02, 0x81, 0x00, 0x00, 0x00, 0x03} save + RESET
    //const unsigned char msg[] = {0xFB, 0x01, 0x03, 0x81, 0x01, 0x1E, 0x01, 0x01};
    //const unsigned char msg[] = {0xFB, 0x01, 0x16, 0x85, 0x9A, 0x9B, 0x9C, 0xAD, 0xAE, 0xAF, 0xB0, 0xC4, 0xC5, 0xCA, 0xCB, 0xCC, 0xAA, 0xAB, 0xAC, 0xC7, 0xC8, 0xC9, 0x80, 0x00 }; //запись USER_PACKAGE
    const unsigned char msg[] = {0xFB, 0x01, 0x03, 0x81, 0x01, 0x05, 0x00, 0x01}; //исполнение user package
    //const unsigned char msg[] = {0xFB, 0x01, 0x05, 0x09};       // просмотр матрицы ориентации
    /*const unsigned char msg[] = {0xFB, 0x01, 0x05, 0x89,
                                 0x00, 0x00, 0x00, 0x00,

                                 0x00, 0x00, 0x80, 0x3f,
                                 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x80, 0x3f,
                                 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x80, 0x3f
                                 };*/
                                                                                                        //const unsigned char msg[] = {0xFB, 0x01, 0x02, 0x81, 0x00, 0x00, 0x00, 0x00};
    char msg_[sizeof(msg)];
    std::memcpy(msg_,msg,sizeof(msg));
    QByteArray data(msg_,sizeof(msg_));
    data.push_back(crc(msg,sizeof(msg)));   //контрольная сумма
    qInfo() << "CRC:" << crc(msg,sizeof(msg));

    /////


    QObject::connect(&port, &QSerialPort::readyRead,
                     [&port](){
        qDebug() << port.bytesAvailable();
        QByteArray message;
        //qDebug() << message;
        QFile fileOut("C:/Users/user/Documents/bins-interaction/bins_log_1.csv");
        fileOut.open(QIODevice::Append | QIODevice::Text);
        QTextStream writeStream(&fileOut);

        QTime time = QTime::currentTime();
        QString textTime = time.toString("hh:mm:ss");

        if (port.bytesAvailable() == 81) //(41)(65)
        {
            message = port.readAll();
            //qInfo() << "reciever recv data ...\n" << QString(message.toHex(':'));
            QByteArray charMessage;
            for(int i = 0; i <= message.length() - 1; i++)
            {
                //printf("0x%02x\t", (unsigned char)message[i]);
                charMessage.push_back((unsigned char)message[i]);
            }
            qDebug() << "\n";
            QStringList messageList = QString(charMessage.toHex(':')).split(':');
//            for(int i = 0; i < messageList.length(); i++)
//            {
//                qDebug() << messageList.at(i);
//            }


//            for (int i = 4; i <= charMessage.length() - 4; i = i + 4)           // просмотр матрицы ориентации
//            {
//                std::memcpy(&test, charMessage.data() + i, sizeof(test));
//                std::cout << "test = " << test << "\n";
//            }
            qDebug() << textTime;
            writeStream << textTime << ";";
            std::memcpy(&heading, charMessage.data()+4, sizeof(heading));
            std::cout << "Heading = " << heading << "\n";
            writeStream << heading << ";";
            std::memcpy(&roll, charMessage.data()+8, sizeof(roll));
            std::cout << "Pitch = " << roll << "\n";
            writeStream << roll << ";";
            std::memcpy(&pitch, charMessage.data()+12, sizeof(pitch));
            std::cout << "Roll = " << pitch << "\n";
            writeStream << pitch << ";";
            std::memcpy(&latitude, charMessage.data()+16, sizeof(latitude));
            std::cout << "Latitude = " << latitude << "\n";
            writeStream << latitude << ";";
            std::memcpy(&longitude, charMessage.data()+24, sizeof(longitude));
            std::cout << "Longitude = " << longitude << "\n";
            writeStream << longitude << ";";
            std::memcpy(&latitudeAccuracy, charMessage.data()+32, sizeof(latitudeAccuracy));
            std::cout << "LatitudeAccuracy = " << latitudeAccuracy << "\n";
            writeStream << latitudeAccuracy << ";";
            std::memcpy(&longitudeAccuracy, charMessage.data()+36, sizeof(longitudeAccuracy));
            std::cout << "LongitudeAccuracy = " << longitudeAccuracy << "\n";
            writeStream << longitudeAccuracy << ";";
            std::memcpy(&headingAccuracy, charMessage.data()+40, sizeof(headingAccuracy));
            std::cout << "headingAccuracy = " << headingAccuracy << "\n";
            writeStream << headingAccuracy << ";";
            std::memcpy(&pitchAccuracy, charMessage.data()+44, sizeof(pitchAccuracy));
            std::cout << "pitchAccuracy = " << pitchAccuracy << "\n";
            writeStream << pitchAccuracy << ";";
            std::memcpy(&rollAccuracy, charMessage.data()+48, sizeof(rollAccuracy));
            std::cout << "rollAccuracy = " << rollAccuracy << "\n";
            writeStream << rollAccuracy << ";";
            std::memcpy(&EAST_VELOCITY, charMessage.data()+52, sizeof(EAST_VELOCITY));
            std::cout << "EAST_VELOCITY = " << EAST_VELOCITY << "\n";
            writeStream << EAST_VELOCITY << ";";
            std::memcpy(&NORTH_VELOCITY, charMessage.data()+56, sizeof(NORTH_VELOCITY));
            std::cout << "NORTH_VELOCITY = " << NORTH_VELOCITY << "\n";
            writeStream << NORTH_VELOCITY << ";";
            std::memcpy(&VERTICAL_VELOCITY, charMessage.data()+60, sizeof(VERTICAL_VELOCITY));
            std::cout << "VERTICAL_VELOCITY = " << VERTICAL_VELOCITY << "\n";
            writeStream << VERTICAL_VELOCITY << ";";
            std::memcpy(&EAST_VELOCITY_Accuracy, charMessage.data()+64, sizeof(EAST_VELOCITY_Accuracy));
            std::cout << "EAST_VELOCITY_Accuracy = " << EAST_VELOCITY_Accuracy << "\n";
            writeStream << EAST_VELOCITY_Accuracy << ";";
            std::memcpy(&NORTH_VELOCITY_Accuracy, charMessage.data()+68, sizeof(NORTH_VELOCITY_Accuracy));
            std::cout << "NORTH_VELOCITY_Accuracy = " << NORTH_VELOCITY_Accuracy << "\n";
            writeStream << NORTH_VELOCITY_Accuracy << ";";
            std::memcpy(&VERTICAL_VELOCITY_Accuracy, charMessage.data()+72, sizeof(VERTICAL_VELOCITY_Accuracy));
            std::cout << "VERTICAL_VELOCITY_Accuracy = " << VERTICAL_VELOCITY_Accuracy << "\n";
            writeStream << VERTICAL_VELOCITY_Accuracy << ";";
            std::memcpy(&STATUS, charMessage.data()+76, sizeof(STATUS));
            std::cout << "STATUS = " << STATUS << "\n";
            writeStream << STATUS << ";";
            //fileOut.flush();
             writeStream << "\n";

            fileOut.close();
            unsigned char checkCrcOfMessage = crc(charMessage,charMessage.length()-1);
            QString crcOfMessage = messageList.at(messageList.length()-1);
            //QByteArray test = charMessage.at(charMessage.length()-1);
            if (crcOfMessage.compare((QString)checkCrcOfMessage))
            {
                qDebug() << "CRC CORRECT; " << "bins crc (Hex) = " << crcOfMessage << "; check crc (Dec) = " << checkCrcOfMessage;
            } else {
                qDebug() << "CRC INCORRECT" << "bins crc (Hex) = " << crcOfMessage << "check crc (Dec) = " << checkCrcOfMessage;
            }

        }

    });

   // Инициализация таймера
    QTimer timer;
    QObject::connect(&timer,&QTimer::timeout,
                     [&port, &data](){
        //qDebug() << "\n\nsending" << QString(data.toHex(':'));
        port.write((data));                  //передача данных
        //port.write(data2);   //матрица ориентации
        //port.write(data3);   //reset+sava запись
        //port.write(data4);   //reset исполнение?
    });

    timer.start(1000); // период 20 мс

    if(port.waitForBytesWritten(3000) == false)
        qDebug() << "errno on write";
    else qInfo() << "ok";

    return a.exec();
}
