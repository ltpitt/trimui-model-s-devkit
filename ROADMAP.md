# 🚀 Trimui Model S DevKit — Roadmap

This roadmap defines the direction of the **Trimui Model S DevKit** project.  
Our goal is to build the **best community resource** for:

- understanding the Trimui Model S  
- modding and customizing it safely  
- creating a full SDK + toolchain for developers to build homebrew
- providing a curated, user‑friendly SD card setup  

This project combines user‑friendly guides with developer‑focused tooling.

---

# 🧭 Vision

## ✔️ For Users
- Clear device documentation  
- Safe modding guides  
- Ready‑to‑flash SD card image with:
  - best emulators  
  - best frontend(s)  
  - clean folder structure  
  - QoL enhancements  

## ✔️ For Developers
- Fully working **toolchain**  
- Fully documented **SDK**  
- Example apps and templates  
- Reverse‑engineering notes  
- Tools to inspect, test and extend the device  

---

# ❓ Toolchain vs SDK

### 🛠 Toolchain
The **toolchain** is everything required to *compile* programs for the device:

- cross‑compiler  
- linkers & binutils  
- sysroot  
- CMake/Make configuration  

This is the build infrastructure.

### 📦 SDK
The **SDK** sits on top and provides:

- headers  
- libraries  
- example projects  
- boilerplate templates  
- helper scripts  
- developer documentation  

The repo today contains **documentation + initial research** — this roadmap will expand it into a full toolchain + SDK.

---

# 🗺️ Project Roadmap

## **Phase 1 — Foundations & Documentation**
> Build the base knowledge for understanding and modding the device.

### 🎯 Goals
- [ ] Device overview (hardware, SoC, RAM, GPU, battery, I/O)  
- [ ] Firmware architecture explanation  
- [ ] Boot sequence documentation  
- [ ] Complete filesystem reference  
- [ ] Description of stock OS behavior  
- [ ] Consolidation of research into `/docs`  

### Community Opportunities
- Teardown photos and measurements  
- Firmware version comparisons  
- Documenting unrecognised binaries/scripts  

---

## **Phase 2 — Toolchain & SDK Infrastructure**
> Build the core developer environment.

### 🎯 Goals
- [ ] Define target architecture & compile settings  
- [ ] Create reproducible toolchain (Docker or scripts)  
- [ ] Extract or build sysroot  
- [ ] Establish SDK structure:
  - `/include`  
  - `/lib`  
  - `/examples`  
  - `/templates`  
- [ ] Provide example apps (hello world, input test, graphics sample)  
- [ ] “Your first Trimui app” tutorial  
- [ ] Optional: VS Code integration  

### Community Opportunities
- Provide example code  
- Test toolchain on multiple OSes  
- Contribute bindings for device APIs  

---

## **Phase 3 — System Exploration & Modding**
> Make the device more flexible and customizable.

### 🎯 Goals
- [ ] Full filesystem map  
- [ ] Document configs (input, display, sound, boot scripts)  
- [ ] Launcher / UI behavior analysis  
- [ ] Safe modding guidelines  
- [ ] Theme customization (fonts, icons, layouts)  
- [ ] Investigate custom launchers  

### Community Opportunities
- Experimental patches  
- UI modding  
- Reverse‑engineering findings  

---

## **Phase 4 — Opinionated SD Card Image**
> A clean, curated, ready‑to‑use SD card setup.

### 🎯 Goals
- [ ] Standard SD card folder layout:
  - `/Emulators`  
  - `/Roms`  
  - `/Bios`  
  - `/Apps`  
  - `/Themes`  
  - `/Saves`  
- [ ] Select and configure **best emulators**  
- [ ] Preinstall best frontend:
  -  GMenuNX
- [ ] Include utility tools  
- [ ] Automatic SD creation script  

### Community Opportunities
- Benchmarking emulators  
- Testing frontend performance  
- Contributing themes  

---

## **Phase 5 — Homebrew Ecosystem**
> Help the community build and share new content.

### 🎯 Goals
- [ ] Create sample homebrew collection  
- [ ] Standard packaging format  
- [ ] Developer submission guide  
- [ ] Showcase page for completed apps  

### Community Opportunities
- Port open‑source games  
- Share homebrew creations  
- Suggest library additions  

---

## **Phase 6 — Advanced / Stretch Goals**
> Ambitious or sheer crazy ideas for the future.

- Community‑maintained alternative firmware (non‑infringing)  
- Kernel builds (if feasible)  
- Live debugging integration  
- Performance profiling tools  
- Full Trimui device emulator/simulator  

---

# 🤝 Contributing

We welcome all contributions — code, research, testing, or documentation.  
Use GitHub issues labeled:

- **good first issue**  
- **discussion**  
- **research needed**  
- **device testing**  

Everyone is welcome, regardless of experience.

---

# ❤️ Thanks

This project exists for the community, driven by curiosity and a passion for open handheld devices.  
Together we can make the Trimui Model S a fantastic playground for learning, hacking, and creating.

🎮✨ Happy modding and building!
