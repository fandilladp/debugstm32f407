#include <Arduino.h>
#include <virtuabotixRTC.h>
#include <HardwareSerial.h>
// Inisialisasi pin (CLK, DAT, RST)
virtuabotixRTC myRTC(PB10, PB11, PA3);
HardwareSerial Serial2(PD6, PD5);
void setup()
{
    // Inisialisasi port serial arduino dengan komputer
    Serial.begin(9600);
    Serial2.begin(9600);
    Serial2.print("tes");
    // penulisan data pertama kali dan disarankan saat transfer
    // ke-2 ini tidak digunakan dikarenakan akan menghapus data
    // sebelumnya
    // myRTC.setDS1302Time(30, 2, 23, 2, 5, 9, 2022);
    // detik, menit, jam, hari dalam seminggu, tanggal, bulan, tahun
    //  00:59:23 "Rabu" 7-September-2017
}

void loop()
{
    myRTC.updateTime();
    Serial2.print("loopnih");
    // memanggil fungsites untuk update data waktu

    // penulisan data pada serial monitor komputer
    Serial2.print("Current Date / Time: ");
    // fungsi penulisan data untuk tanggal
    Serial2.print(myRTC.dayofmonth);

    // penulisan data "/" sebagai separator
    Serial2.print("/");

    // fungsi penulisan data untuk bulan
    Serial2.print(myRTC.month);

    // penulisan data "/" sebagai separator
    Serial2.print("/");

    // fungsi penulisan data untuk tahun
    Serial2.print(myRTC.year);

    // penulisan data untuk jarak
    Serial2.print(" ");

    // fungsi penulisan data untuk jam
    Serial2.print(myRTC.hours);

    Serial2.print(":");

    // fungsi penulisan data untuk menit
    Serial2.print(myRTC.minutes);

    Serial2.print(":");

    // fungsi penulisan data untuk detik
    Serial2.println(myRTC.seconds);
    delay(1000);
}
