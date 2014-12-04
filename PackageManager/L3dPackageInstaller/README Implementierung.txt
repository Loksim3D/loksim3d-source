*********
* Ziele *
*********

- Deinstallation von Packages möglich
- Anzeige von installierten Dateien (aufgeteilt in schreibgeschützte und nicht schreibgeschützte)
- Abbrechen jederzeit möglich (NTFS, Vista+)
- "All-or-Nothing" Prinzip - auch bei Abstürzen oder Stromausfällen (NTFS, Vista+)
- Multi-Package Installation
- Performance-Verbesserung
- Möglichkeit detailliert auswählen zu können welche Dateien installiert werden (aufgeteilt in schreibgeschützte und nicht schreibgeschützte)


*******************
* Implementierung *
*******************

--- lhstd und lhWinAPI Projekte --
Diese beiden Projekte bieten ein paar hilfreiche Methoden. lhstd vereinfacht im Prinzip die Anwendung der C++ STL. lhWinAPI der Win32-API

-- Konzept --
Bei der Installation eines Package werden Metadaten zum Package (Checksum, Readme, Pfad, Backuppfad) in eine SQLite DB geschrieben (Packages Tabelle)
Außerdem wird jede Datei im Package in die DB geschrieben (Files Tabelle). Für jede hinzugefügt Datei in dieser Tabelle wird vermerkt, ob die Datei vorher schon im Loksim-Verzeichnis existiert hat. 
Falls ja, darf diese Datei bei Deinstallationen nicht durch den Package-Manager gelöscht werden
Jede Datei im Loksim-Verzeichnis existiert maximal einmal in der Files Tabelle. Die Zuordnung welche Datei
durch welches Package verwendet wird, erfolgt mittels der Dependencies Tabelle.
Aus Performance-Gründen existiert in der Files Tabelle die Spalte "UsageCount" welche speichert, wieviele Packages diese Datei verwenden. Dieser Counter wird durch Trigger in der Dependencies Tabelle
am aktuellen Stand gehalten.

Bei der Deinstallation wird nun zum einen die Package-Information in der Packages Tabelle + alle Dependencies dieses Packages gelöscht. 
Alle Dateien in der Files Tabelle wo UsageCount nun (durch den Delete-Trigger) 0 ist, können vom Dateisystem gelöscht werden. Anschließend werden all diese Dateien auch aus dem Files Tabelle gelöscht.

Ein "Undo-Installation" spielt darüber hinaus auch noch die gesicherten Dateien aus dem Backup Verzeichnis ins Loksim-Verzeichnis zurück.

Sämtliche Operationen werden im Hintergrund ausgeführt. Wenn möglich wird die Arbeit auf mehrere Thread verteilt. Beides wird mittels der Concurreny Runtime (http://msdn.microsoft.com/en-us/library/dd504870.aspx)
umgesetzt.
Es hat sich herausgestellt, dass das "Bottleneck" die DB-Operationen sind: Diese können nicht auf mehrere Threads aufgeteilt werden, da eine Transaktion bei SQLite immer in einem Thread ausgeführt
werden muss

Info zu Zeichensatz in ZIP-Dateien:
Prinzipiell ist die unzip.cpp Datei vollständig aus externer Quelle. Jedoch wurde beim Dekodieren der Dateinamen der zeichensatz von UTF-8 auf OEM 437 umgestellt: 
Obwohl in den Package-Dateien eigentlich alle Dateinamen kodiert vorliegen, hat sich in der Praxis herausgestellt, dass manche Packages deutsche Umlaute in OEM 437 Kodierung enthalten => 
Dekodierung mittels UTF-8 würde diese Packages "unbrauchbar" machen

Lukas Haselsteiner

Zusatz März 2013
Inzwischen bin ich mit der Umsetzung des PackageManager nicht mehr so richtig glücklich. Eine bessere Aufteilung der einzelnen Komponenten wäre sehr gut gewesen. Inzwischen gibt es zu viele Abhängigkeiten
und das Programm ist zu komplex geworden.
Das war wohl mein gezahltes "Lehrgeld", bei größeren Erweiterungen sollte man m.E. darüber nachdenken ob vorher ein umfangreiches Refactoring zur Vereinfachung des Codes durchgeführt werden sollte
