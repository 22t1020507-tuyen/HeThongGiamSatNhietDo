# 🌿 Hệ Thống giám sát nhiệt độ, độ ẩm và điều khiển tưới tiêu tự động 

Dự án này mô phỏng một hệ thống tưới cây và sưởi ấm tự động sử dụng vi điều khiển ESP32, giao tiếp qua giao thức MQTT và quản lý trên giao diện Node-RED. Hệ thống hỗ trợ cả chế độ tự động hóa hoàn toàn và điều khiển thủ công từ xa.

---

## 🚀 Các tính năng chính

* **Đo lường thời gian thực:** Thu thập dữ liệu nhiệt độ, độ ẩm không khí và độ ẩm đất.
* **Hiển thị tại chỗ:** Màn hình LCD 16x2 hiển thị thông số môi trường và trạng thái hoạt động (AUTO/MANUAL, trạng thái kết nối).
* **Chế độ tự động (AUTO):**
    * Tự động bật máy bơm khi đất khô (độ ẩm < 40%) hoặc thời tiết khắc nghiệt (Nhiệt độ > 35°C, độ ẩm không khí < 40% và độ ẩm đất < 70%). Tắt khi độ ẩm đất > 70%.
    * Tự động bật đèn sưởi công suất tối đa (100%) khi nhiệt độ < 20°C, và bật mờ khi từ 20°C - 25°C.
* **Chế độ thủ công (MANUAL):** Cho phép người dùng bật/tắt máy bơm và điều chỉnh độ sáng đèn (PWM) từ xa thông qua giao diện Node-RED UI.
* **Cảnh báo trạng thái:** Kích hoạt còi báo động (Buzzer) khi máy bơm đang hoạt động.
* **Giao thức MQTT:** Kết nối ổn định thông qua HiveMQ Cloud với bảo mật TLS/SSL.

---

## 🛠 Linh kiện sử dụng

| Tên linh kiện | Chức năng | Chân kết nối ESP32 |
| :--- | :--- | :--- |
| **ESP32** | Vi điều khiển trung tâm (Kết nối WiFi & MQTT) | N/A |
| **DHT22** | Cảm biến Nhiệt độ và Độ ẩm không khí | `Pin 14` |
| **Potentiometer** | Giả lập Cảm biến độ ẩm đất (0-4095) | `Pin 34 (Analog)` |
| **Relay Module** | Điều khiển máy bơm nước 5V/12V | `Pin 2` |
| **LCD 16x2 I2C** | Màn hình hiển thị thông số và trạng thái | `SDA: 21`, `SCL: 22` |
| **LED Vàng** | Giả lập hệ thống Đèn sưởi ấm (điều khiển băm xung PWM) | `Pin 5` |
| **Buzzer/LED Đỏ** | Báo hiệu bằng âm thanh/ánh sáng khi bơm chạy | `Pin 4` |

---

## 💻 Hướng dẫn mô phỏng trên Wokwi

1.  **Truy cập Wokwi:** Mở trình duyệt và truy cập vào [Wokwi ESP32 Simulator](https://wokwi.com/esp32).
2.  **Thiết lập sơ đồ mạch:** * Thêm các linh kiện như danh sách trên vào vùng không gian làm việc.
    * Thực hiện nối dây đúng với các chân đã định nghĩa trong bảng linh kiện.
    * *Lưu ý:* Thêm file thư viện `diagram.json` nếu bạn muốn giữ nguyên vị trí linh kiện đã lưu.
3.  **Cài đặt thư viện (Library Manager):**
    Vào tab "Library Manager" (biểu tượng sách) và cài đặt các thư viện sau:
    * `DHT sensor library for ESPx` (hoặc `DHT sensor library`)
    * `LiquidCrystal I2C`
    * `PubSubClient`
4.  **Chạy mã nguồn:**
    * Copy toàn bộ mã nguồn `C++` (.ino hoặc main.cpp) dán vào cửa sổ soạn thảo.
    * Nhấn nút **Play** (màu xanh lá) để bắt đầu mô phỏng. Màn hình LCD sẽ báo kết nối WiFi (Wokwi-GUEST) và sau đó là kết nối tới HiveMQ.

---

## 🌐 Hướng dẫn chạy Node-RED

Hệ thống điều khiển qua Node-RED sử dụng các node MQTT và Dashboard UI.

### 1. Yêu cầu chung
* Đã cài đặt [Node-RED](https://nodered.org/docs/getting-started/) và đang chạy tại `http://localhost:1880`.
* Đã cài đặt palette cho Dashboard: Vào **Manage palette** -> Tìm và cài `node-red-dashboard`.

### 2. Cấu hình MQTT Broker (HiveMQ Cloud)
Khi kéo các node `mqtt in` hoặc `mqtt out` ra màn hình, bạn cần cấu hình Server (Broker) như sau:
* **Server:** `31e313925d0b4a6a965c8c890db0d38f.s1.eu.hivemq.cloud`
* **Port:** `8883`
* **Tick chọn:** `Enable secure (SSL/TLS) connection`
* **Tab Security:** Nhập Username (`tuyen2808`) và Password (`Tuyen28082004@`).
* **Client ID:** Để trống (để tự cấp) hoặc điền một ID duy nhất.

### 3. Danh sách MQTT Topics cấu hình

**Đọc dữ liệu (Sử dụng node `mqtt in`):**
* `adminpc/tuoicay/nhietdo`: Nhận dữ liệu nhiệt độ.
* `adminpc/tuoicay/doam`: Nhận dữ liệu độ ẩm không khí.
* `adminpc/tuoicay/doamdat`: Nhận dữ liệu độ ẩm đất.
* `adminpc/tuoicay/trangthaibom`: Nhận trạng thái thực tế của máy bơm (ON/OFF).

**Gửi lệnh điều khiển (Sử dụng node `mqtt out`):**
* `adminpc/tuoicay/bom`: Gửi lệnh điều khiển bơm (payload: `"ON"`, `"OFF"`, `"AUTO"`). Cấp payload từ các UI Button.
* `adminpc/tuoicay/pwm`: Gửi lệnh độ sáng đèn (payload: `0 - 255`). Cấp payload từ UI Slider.

### 4. Triển khai (Deploy)
* Nối các node `mqtt in` vào các UI Gauge / UI Text để hiển thị dữ liệu.
* Nối các UI Button / UI Slider vào các node `mqtt out` để gửi lệnh.
* Nhấn nút **Deploy** (màu đỏ ở góc phải). Nếu các node MQTT hiện chấm xanh lá cây (**connected**), bạn đã kết nối thành công.
* Truy cập `http://localhost:1880/ui` để xem và điều khiển giao diện Dashboard.