🚖 SwiftGo – Ride Sharing Management System (C++)
-A fully functional **console-based ride-sharing system** built using **custom data structures and OOP principles** in C++.


🌟 Overview

SwiftGo simulates a real-world ride-hailing platform (like Uber/Careem) with:

* Users booking rides
* Drivers accepting and completing rides
* Admin managing the system
* Smart ride assignment logic


⚙️ Key Features

 👤 User Portal

* Create account with validation
* Book rides with:
  - Pickup & drop-off
  -Distance input
  -Preferred fare system
* View ride history
* Track pending requests


🚗 Driver Portal

* Driver registration with distance preference
* Accept ride requests manually
* Complete rides & update availability
* Rating system (users rate drivers)


🧠 Smart Ride Assignment

* Auto-assigns rides using custom scoring:
  - Driver distance preference
  - Driver rating
* Uses manual scanning algorithm (no STL priority queue)


🛠️ Admin Portal

* View all users & drivers
* Monitor ride history
* Undo last assignment (if not rated)

📊 System Dashboard

* Total users
* Active drivers
* Pending ride requests
* Real-time system stats

🧩 Custom Data Structures

Instead of STL containers:

🔹 MyStack → Undo functionality
🔹 MyQueue → FIFO structure
🔹 MyDeque → Ride request handling


🧪 Concepts Used

* Object-Oriented Programming (OOP)
* Abstraction & Inheritance
* File-less simulation (in-memory system)
* Custom Data Structures
* Input Validation & Error Handling
* Algorithmic decision making


💡 Highlight Features

✔ Preferred Fare System
✔ Driver Rating Algorithm
✔ Undo Assignment Feature
✔ Custom-built Data Structures
✔ Multi-role system (User / Driver / Admin)


 🖥️ Technologies Used

* C++
* STL (vector, list for support)
* Custom Containers
* Console UI with ANSI Colors
  

📌 Future Improvements

* File handling (persistent storage)
* GUI version
* Map-based distance calculation
* Real-time driver tracking

👨‍💻 Author

bismah195

⭐ Show Some Love

If you like this project, consider giving it a ⭐ on GitHub!
