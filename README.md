# Sistem Monitoring Penggunaan Air Berbasis IoT

## 👥 Anggota Kelompok
| Nama | NIM | Peran |
|---|---|---|
| Quratta A’yuni | 2309106001 | Ketua |
| Nur Juzieatul Alifah | 2309106040 | Anggota |
| Tiara Kasma Wati Putri | 2309106080 | Anggota |

---

## 📌 Judul Projek Akhir
**Sistem Monitoring Penggunaan Air Berbasis IoT**

---

## 📖 Deskripsi
Sistem Monitoring Penggunaan Air Berbasis IoT merupakan sistem yang dirancang untuk memantau volume dan penggunaan air secara real-time menggunakan teknologi Internet of Things (IoT). Sistem ini menggunakan ESP32 sebagai mikrokontroler utama yang terhubung dengan sensor ultrasonik HC-SR04 untuk mengukur tinggi air pada tandon dan sensor water level untuk mendeteksi kondisi penuh.

Data yang diperoleh dari sensor diproses untuk menghitung volume air serta penggunaan air harian. Informasi tersebut kemudian dikirim ke platform Antares menggunakan protokol MQTT sehingga dapat dipantau secara online.

Sistem juga dilengkapi dengan:
- Servo motor untuk kontrol buka/tutup kran otomatis maupun manual
- Telegram Bot untuk notifikasi air rendah
- NTP Client untuk sinkronisasi waktu dan reset penggunaan harian otomatis
- MQTT EMQX Broker untuk kontrol servo secara real-time

Projek ini bertujuan membantu pengguna dalam memonitor penggunaan air secara lebih efektif, efisien, dan real-time.

---

## 👨‍💻 Pembagian Tugas per Individu

### Quratta A’yuni
- Merangkai alat dan komponen IoT
- Membuat program utama sistem
- Melakukan troubleshooting coding dan integrasi sistem

### Nur Juzieatul Alifah
- Merangkai alat
- Membuat hampir seluruh tampilan aplikasi Kodular
- Membuat logika block Kodular
- Melakukan troubleshooting block Kodular

### Tiara Kasma Wati Putri
- Membantu proses perangkaian alat
- Menyusun dan menulis laporan projek

---

## 🔧 Komponen yang Digunakan
- ESP32
- Sensor Ultrasonik HC-SR04
- Sensor Water Level
- Servo Motor
- LED Indikator
- Kabel Jumper
- Smartphone
- Platform Antares IoT
- MQTT Broker EMQX
- Telegram Bot
- Arduino IDE

---

### Desain Board
Board schematic dirancang menggunakan:
- Fritzing
<img width="2795" height="2023" alt="iot" src="https://github.com/user-attachments/assets/0bbea81a-75c3-44df-a2dd-10dd117a3ee8" />

> Tambahkan gambar schematic hasil rancangan kalian pada bagian ini.

## 🔌 Board Schematic

### Konfigurasi Pin ESP32
| Komponen | Pin ESP32 |
|---|---|
| TRIG HC-SR04 | GPIO 5 |
| ECHO HC-SR04 | GPIO 18 |
| Servo Motor | GPIO 13 |
| Water Level Sensor | GPIO 34 |
| LED Indikator | GPIO 2 |

### Wiring Diagram
- HC-SR04 VCC → 5V ESP32
- HC-SR04 GND → GND ESP32
- Water Level VCC → 5V ESP32
- Water Level GND → GND ESP32
- Servo VCC → 5V ESP32
- Servo GND → GND ESP32
- LED (+) → GPIO 2
- LED (-) → GND
