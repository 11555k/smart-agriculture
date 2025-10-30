### 🌱 Smart Agriculture System

A sustainable smart agriculture project built around an **ESP32**, **soil moisture sensor**, and **solar-powered system**. It features a **custom PCB** and a **homemade Power Management System (PMS)** that manages charging and power delivery from a solar panel and lithium-ion battery. The goal is to create a reliable, low-cost, and energy-efficient system for automatic irrigation and smart farming.

---

## ⚙️ Features

* Real-time soil moisture sensing
* Automatic irrigation control
* Solar-powered with lithium-ion battery storage
* Custom homemade PMS for stable power management
* Custom PCB for compact and efficient design
* GUI for live monitoring
* Fully offline operation (no need for constant Wi-Fi)

---

## 🔋 Power System

* **Solar Panel:** 6V–12V depending on available sunlight
* **Battery:** 18650 lithium-ion cell
* **Homemade PMS:** Manages solar charging, over-discharge protection, and regulated output to 5V


---

## 🧠 Components Used

* ESP32 Development Board
* Soil Moisture Sensor
* Relay Module (for pump control)
* Solar Panel
* 18650 Lithium-ion Battery
* Custom PMS Circuit
* Custom PCB

---

## 🧩 PCB & Schematic

Includes:

* PMS section
* ESP32 header
* Soil sensor interface
* Relay driver circuit
* Test points for debugging and current monitoring

*(Add images if available)*
`![PCB Layout]![WhatsApp Image 2025-10-30 at 23 47 28_00d2993d](https://github.com/user-attachments/assets/56eb36cd-de8d-4a78-bd95-2d478cccb7de)

`
`![Schematic](![WhatsApp Image 2025-10-30 at 23 37 55_8db17a8a](https://github.com/user-attachments/assets/caf2f2be-b3d3-42aa-a5d8-1de20dc19f8c)
)`
![WhatsApp Image 2025-10-30 at 23 01 42_797ce561](https://github.com/user-attachments/assets/f3b813ac-a374-43cf-8ad6-8fc972d96f30)

![WhatsApp Image 2025-10-30 at 23 04 20_ce58f6c8](https://github.com/user-attachments/assets/44adcd94-0668-4609-b24c-0004176e68a2)

![WhatsApp Image 2025-10-30 at 23 01 47_db55de3d](https://github.com/user-attachments/assets/1001a6dc-4dbe-4f55-bf44-2967d564f2f6)

## GUI
* HTML, CSS , JAVASCRIPT , AJAX


https://github.com/user-attachments/assets/f95c4ca3-cc9d-40af-b9c0-b75ac4cffb38


---

## 🔨 How to Build It Yourself

1. **Assemble PMS:** Connect solar panel → PMS → lithium-ion battery.
2. **Power ESP32:** Use PMS regulated 5V output to power ESP32 VIN.
3. **Connect Components:**

   * Soil sensor → Analog pin
   * Relay → Digital pin
   * Pump → Relay output
4. **Upload Code:** Flash the firmware from `/software` using Arduino IDE.
5. **Monitor & Test:** Check soil readings and verify auto pump control.

---


**Author:** [Marwan mostafa]
**Tags:** ESP32, IoT, Embedded Systems, Renewable Energy, Smart Agriculture, PCB Design
++++
