<<<<< STM32 Cross-Compile + Deploy + Debug in CQcreator >>>>>

sudo apt install gdb-multiarch
sudo apt install openocd

Debug Sever Starten, für die jeweilige MCU:
openocd -f /usr/share/openocd/scripts/interface/stlink.cfg -f /usr/share/openocd/scripts/target/stm32f4x.cfg

In QTcreator:
Kits-->STM32 anlegen
dann dort bei CMake config folgendes anfügen (Toolchain Setup)
-DCMAKE_TOOLCHAIN_FILE:FILEPATH=/home/ksl/.config/cmake/STM32-toolchain.cmake

In QTcreator, für Proggen wenn man RUN/DEBUG drückt:
In Project-->Build&Run-->DeploySettings einen additional build step einfügen:
cmd: openocd
arg: -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program /home/ksl/DevLocal/builds/foo/STM32-test.elf  verify reset exit"


----------------

Du brauchst im Prinzip drei Dinge: ein ARM‑Kit in Qt Creator, ein Bare‑Metal/OpenOCD‑Target und ein Debug‑Run‑Config, das dein ELF flasht und anhängt.
habr
+2

0. Voraussetzungen klären
Installiert sein sollten: arm‑none‑eabi‑Toolchain, OpenOCD, STM32Cube‑Projekt (idealerweise CMake), Qt Creator mit Bare‑Metal‑Plugin.
github
+2

Physik: STM32 über ST‑LINK (on‑board oder extern) per USB am Rechner.
github
+1

Wenn du mir konkrete MCU (z.B. STM32G431), Board und OS nennst, kann ich die OpenOCD‑Zeile unten genau anpassen.

1. Bare‑Metal‑Plugin in Qt Creator aktivieren
Qt Creator öffnen.

Menü: Help → About Plugins (oder Hilfe → Über Plugins).
bartslinger
+1

Unter „Device Support“ das BareMetal‑Plugin anhaken.

Qt Creator neu starten.
github
+1

2. ARM‑Compiler und GDB eintragen
Menü: Tools → Options → Build & Run → Reiter Compilers.
bartslinger
+1

„Add“ → „GCC“ → „C“ wählen, Name z.B. ARM GCC, Pfad zu arm-none-eabi-gcc setzen.
github
+1

Dasselbe für C++: „Add“ → „GCC“ → „C++“, Pfad zu arm-none-eabi-g++.
github
+1

Reiter Debuggers: „Add“, Pfad zu arm-none-eabi-gdb-py setzen (wichtig: Python‑Variante).
github
+2

Qt Creator sollte bei beiden erkennen, dass es sich um arm‑baremetal‑ABIs handelt.
github
+1

3. OpenOCD‑Server und Bare‑Metal‑Device anlegen
Menü: Tools → Options → Devices → Reiter Bare Metal.
habr
+2

„Add“ → OpenOCD (oder „Add OpenOCD server“) auswählen.
habr
+1

Executable / OpenOCD: Pfad zu openocd.

Root scripts directory: z.B. /usr/share/openocd/scripts (Linux).
github
+1

Additional arguments: passende Interface‑ und Target‑Configs, z.B.:
-f interface/stlink-v2-1.cfg -f target/stm32f4x.cfg (Beispiel F4‑Board).
bartslinger
+1

Übernehmen.

Im gleichen Dialog ein Bare Metal Device anlegen: „Add → Bare Metal Device → Start Wizard“, dann den eben definierten OpenOCD‑Server als „Debug server provider“ auswählen.
habr



Damit hat Qt Creator ein „Gerät“, auf das er deployen und an dem er debuggen kann.
github
+1

4. ARM‑Bare‑Metal‑Kit anlegen
Tools → Options → Build & Run → Reiter Kits.
github
+1

„Add“ (oder bestehendes „Desktop“-Kit duplizieren) und z.B. STM32 ARM Bare Metal nennen.
github
+1

Felder setzen:
github
+1

Compiler C: ARM GCC (arm‑none‑eabi‑gcc).

Compiler C++: ARM G++.

Debugger: arm-none-eabi-gdb-py.

Device type: „Bare Metal Device“.

Device: das in Schritt 3 angelegte Bare‑Metal‑Device (OpenOCD).
habr
+1

Falls dein Projekt mit CMake läuft: CMake Tool auswählen und ggf. dein Toolchain‑File über CMake‑Preset oder Extra-Argument (-DCMAKE_TOOLCHAIN_FILE=...) eintragen.
github
+1

Kit speichern.

5. STM32‑Projekt in Qt Creator öffnen und Build konfigurieren
File → Open File or Project und dein CMakeLists.txt oder das CMake‑Projekt auswählen.
github



Bei der Kit‑Auswahl dein STM32 ARM Bare Metal‑Kit wählen.
github
+1

CMake konfigurieren lassen; prüfen, dass im CMake‑Output arm-none-eabi-gcc auftaucht, nicht der Host‑GCC.
github



Build‑Verzeichnis anlegen lassen, dann mit Ctrl+B bauen.
github



Ergebnis sollte eine .elf im Build‑Ordner sein; die brauchst du gleich zum Flashen/Debuggen.
bartslinger
+1

6. Run/Deploy‑Konfiguration (Flashen)
Links in Qt Creator auf „Projects“ klicken, oben dein STM32‑Kit wählen.
bartslinger
+1

Reiter Run.

„Add“ → Run Configuration → Bare Metal → „Run on hardware via GDB server / OpenOCD“ (Name kann leicht variieren).
bartslinger



Dort:
habr
+2

GDB executable: arm-none-eabi-gdb-py.

Executable on device: vollständiger Pfad zur erzeugten .elf.

Debug server: dein OpenOCD‑Server / Bare‑Metal‑Device.

GDB additional options optional (z.B. -ex "monitor reset halt").

Viele Anleitungen starten OpenOCD extra im Hintergrund; mit aktueller Bare‑Metal‑Integration kann Qt Creator OpenOCD beim Debugstart selbst starten, wenn du das Device richtig zuordnest.
habr
+2

7. Debuggen aus der IDE
ST‑LINK anstecken, Zielboard mit Strom versorgen.
github
+1

In Qt Creator oben dein STM32‑Kit und die neue Run‑Config auswählen.
github
+1

„Start Debugging“ (grüner Käfer) klicken.
bartslinger
+1

Qt Creator baut, startet OpenOCD, flasht die .elf und verbindet GDB – du kannst Breakpoints setzen, Step‑Into, Register/Peripherals ansehen.
