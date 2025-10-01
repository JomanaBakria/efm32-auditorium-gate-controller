# 🎛️ EFM32 Auditorium Gate Controller  

An **EFM32-based embedded system** that controls an auditorium gate, counts people entering/exiting, and displays available seats in real time.  
The system integrates **Flex sensors, PWM-controlled motors, LCD display, and Bluetooth communication** to provide a reliable solution for seat management.  

---

## 🎯 Objectives
- Control auditorium entry and exit using a **motorized turnstile** driven by PWM.
- Track occupancy with **Flex sensors** at the entrance and exit.
- Display the number of **available seats** on the LCD in real time.
- **Prevent entry when the auditorium is full**:
  - If no seats are available → the gate does not rotate and entry is blocked.
  - The person must wait until a seat becomes available.
  - Once someone exits and a seat is freed → entry is automatically enabled.
- Provide manual override via **buttons** for testing and control.
- Send live updates over **Bluetooth** (available seats, PWM level, motor direction, etc.).

---

## ✨ Features
- **EFM32PG28 MCU** with hardware PWM  
- **2× Flex sensors** (entry & exit detection)  
- **DC motor control** (forward/backward with PWM speed control)  
- **Segment LCD display** (remaining seats)  
- **Bluetooth (EUSART1 @ 9600 baud)** for status messages  
- **Manual override buttons**  

---

## 🛠️ Hardware
- **Motor DIR pins:** PD6 / PD9  
- **PWM output:** PD11 (TIMER0 CC0)  
- **Entry sensor:** PD13  
- **Exit sensor:**  PD10  
- **Buttons:** PB1 (Button0), PB6 (Button1)  
- **Bluetooth:** PD14 (TX) → HC-06 RX, PD15 (RX) ← HC-06 TX  

---

## 🚀 Getting Started
1. Open **Simplicity Studio** → create new project for **EFM32PG28**.  
2. Replace `src/main.c` with [`firmware/src/main.c`](firmware/src/main.c).  
3. Build and flash to the board.  
4. Connect via Bluetooth @ 9600 baud to monitor updates.  

---

## 📷 Media
Here are some pictures of the physical project:

![Front View](./media/image%201.jpeg)  
![Top View](./media/image%202.jpeg)  

---

## 🎥 Demo Video
👉 [Watch on YouTube](https://youtu.be/kVEKkczSuOg?si=OT4P-HmoG5x-mjeY)  

---

## 📜 License
This project is released under the [MIT License](LICENSE).
