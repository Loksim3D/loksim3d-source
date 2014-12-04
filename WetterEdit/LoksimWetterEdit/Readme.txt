******************************
* Kompilieren / Bibliotheken *
******************************

* Zum Kompilieren der Effekte muss das "Shader Effects BuildTask and Templates" Package installiert werden (http://wpf.codeplex.com/releases/view/14962) 
(Es reicht eine Installation des enthaltenen Package. Die Template Dateien müssen nicht unbedingt ins entsprechende Verzeichnis kopiert werden)

Dokumentation / Projektseiten zu den verwendeten Zusatzbibliotheken:
* WindowsAPI CodePack http://archive.msdn.microsoft.com/WindowsAPICodePack
	Vista-Dateidialoge
* WPF Extended Toolkit: http://wpftoolkit.codeplex.com/documentation
	DateTime Control
* WPF toolkit: http://wpf.codeplex.com/releases/view/40535
	AutoCompleteBox (Eingabe von Texturen); Dieses Control ist laut Projektseite noch nicht 100%ig stabil, funktioniert aber mE schon jetzt ganz ordentlich



******************
* Grobes Konzept *
******************

Dieser Teil beschreibt auch einige Konzepte und Bibliotheken aus C# / .NET
Die Beschreibung ist aber auf keinen Fall ausreichend, für 100%iges Verständnis muss man wahrscheinlich die Schlagworte explizit "nachgoogeln"

-------
- WPF -
-------
Der Loksim WetterEdit verwendet für die Oberflächen die Windows Presentation Foundation WPF: Grob gesagt ist dies die neueste GUI-Bibliothek für Desktop-Anwendungen von Microsoft. 
Und die erste welche nicht auf native Betriebssystem Controls aufbaut, sondern  praktisch alles selber zeichnet. Hatte lange Zeit den Ruf recht langsam zu sein, aber
inzwischen hat sich das verbessert und mit VisualStudio 2010 hat Microsoft selbst erstmals ein sehr großes Programm mit WPF umgesetzt
"Vorgänger" (oder alternative Möglichkeit) in C# für GUIs ist "Windows Forms". Windows Forms sind im Prinzip C# Wrapper für die nativen Windows Controls. 

Der große Vorteil (für mich) von WPF ist, dass die Oberflächenbeschreibung nicht im Code sondern mit XAML (XML-Dialekt) Dateien erfolgt. VisualStudio 2010 bietet dafür einen Editor,
man kann diese Dateien aber auch in Expression Blend (Microsoft Designwerkzeug) bearbeiten.

- Data Binding -
Die Verknüpfung von Daten mit den GUIs kann man in WPF (aber auch schon Windows Forms) mit "DataBinding" erledigen. Im Falle von WPF bedeutet dies, dass man in der Oberflächen-
beschreibung (XAML) bei den jeweiligen Controls definiert, welche Daten dort angezeigt werden sollen (Name der C# Property) und alles weitere wird von selbst erledigt.
Wenn der User die Daten im GUI ändert, wird automatisch die dahinterliegende Property des "Daten-Objekts" geändert. Diese Bindings sind recht flexibel, man kann zB auch
Validatoren (Überprüfen der Benutzereingaben) oder Konverter (Konvertierung von Werten) angeben.

- MVVM -
Im speziellen kommt beim WetterEdit für die Darstellung der Daten das MVVM-Prinzip (Model - View - View Model) zum Einsatz. Dies ist eine Abwandlung des MVC 
(Model - View - Controller) Prinzips.
Unter Model versteht man die "Datenobjekte". Dies sind normale Klassen welche die Daten kapseln und über Properties bereitstellen. Zusätzlich können (im Fall des WetterEdit
ist dies der Fall) sie das INotifyPropertyChanged Interface implementieren. Dieses Interface schreibt eine Event vor, welches dazu dient, dass andere Klassen einen 
Event Listener registrieren können und damit mitbekommen, wann sich die Daten geändert haben. Setzt man DataBinding im GUI ein, wird das GUI automatisch aktualisiert, falls sich
die Daten geändert haben.
Die View übernimmt die eigentliche Darstellung der Daten. Im Falle von WPF ist dies im Idealfall nur die XAML Datei welche mit Hilfe von DataBindings die Daten anzeigt / 
aktualisiert. Hin und wieder ist es nötig / einfacher wenn man dazu auch noch eine "Code Behind" Datei einsetzt. Dies ist im Prinzip eine Klasse zur XAML-Datei, in welcher
mit Hilfe von Code GUI-Aktionen durchgeführt werden können
Die Schnittstelle zwischen GUI und Model übernimmt das "View Model". Dieses stellt die nötigen Daten für die View bereit, ist aber so implementiert dass man die View ganz
einfach tauschen könnte. Im MVVM Modell informiert die View das ViewModel mit Hilfe von "Commands" über Events (zB Button-Click). Grob gesprochen bietet das zugehörige
ICommand Interface zwei Methoden: Eine Methode zum Ausführen des Commands und eine Methode welche zurückgibt ob der Command gerade ausgeführt werden kann

Im Prinzip sollten die ViewModels niemals irgendwelche Dialoge (zB MessagesBoxen) anzeigen oder Annahmen über die Views machen. Im WetterEdit ist dies derzeit noch nicht
100%ig so, zB öffnen die ViewModels Dialog für Datei Öffnen oder Fehlermeldungen.


---------------
- Architektur -
---------------

- Models -

"Ausgangspunkt" für die Umsetzung ist der "FileWrappers" Namespace (Models im MVVM Sinne): Dort werden die Loksim-Dateien gekapselt. Einlesen / Speichern der Dateien wird dabei mit 
System.Xml.Linq Klassen (Klassennamen X***) erledigt (mE eine der "schönsten" Möglichkeiten überhaupt mit XML im Code zu arbeiten). Diese "X-Klassen" werden jedoch
nur zum Einlesen / Speichern verwendet, ansonsten liegen die Daten in "normalen" Properties. Es ist überlegenswert ob dies überhaupt nötig ist, oder ob man die Daten
nicht die ganze Zeit in der "XDocument" Klasse lassen könnte und in den jeweiligen Properties direkt auf die XML-Elemente zugreift. Ist aber nicht so essentiell, da
dies die Schnittstelle zu den Models nicht ändern würde.
"Neuste Erweiterung" ist, dass auch sämtliche Teile der Dateien (zB Zeitbereich/TimeRange bei den WetterDateien) nach XML konvertierbar sind, bzw von XML eingelesen werden können.
Dies wird für Copy&Paste verwendet (kopiert man einen Teilbereich wird der entsprechende XML-Ausschnitt in die Zwischenablage gelegt)

Für Undo/Redo ist es eventuell noch notwendig, dass sämtliche Klassen welche "Datei-Teile" repräsentieren eine Referenz zur Parent-Datei abspeichern. Auch das würde das Interface
zu den Models nicht oder nur minimal (erstellen von Datei-Teilen) ändern

Oft verwendet ist auch die "L3dFilePath" Klasse: Diese kapselt einen "Loksim-Pfad". Der Pfad wird hierbei durch den absoluten Pfad und eventuell eine "Parent-Datei" repräsentiert.
Beim Erstellen / Einlesen von Pfaden wird also immer sofort in einen absoluten Pfad konvertiert. Da die Information von welcher Datei diese andere - durch die L3dFilePath 
Klasse repräsentierte Datei- referenziert wird, manchmal wichtig ist, wird dies in der ParentFile Property abgespeichert.
L3dFilePath Klassen sind nicht veränderbar, dies erleichtert DataBinding

- Start-Punkt -
Der WetterEdit ist als "SingletonInstance" Anwendung implementiert: D.h. ein User kann den LoksimEdit nur 1x starten (auch wenn mehrere Fenster offen sind, sind diese alle durch
eine Anwendungs-Instanz gemanaged.
Dafür wurde der Startpunt der Anwendung abgeändert, er ist nun in der App Klasse zu finden. Die Implementierung der notwendigen Methoden damit die Anwendung nur 1x gestartet
werden kann, aber trotzdem Parameter einer vom User neu gestarteten Instanz an die laufende Instanz weitergeben werden, ist von einem MS-Mitarbeiter-Blog (SingleInstance Klasse)

- MainWindow -
Im "Lebenszyklus" der Anwendung ist die zweite wichtige Klasse die MainWindow Klasse. Diese ist nicht im MVVM Prinzip implementiert, sondern alle wichtigen Aktionen werden
in der Code-Behind Datei ausgeführt.
Das MainWindow stellt das Menü dar und erledigt das Öffnen / Laden von Dateien. Außerdem hat das MainWindow eine statische Liste von allen offenen Fenstern. Dies ermöglicht
dem MainWindow, dass Dateien die schon offen sind nicht ein zweites Mal geöffnet werden, sondern stattdessen das Fenster welches diese Datei anzeigt in den Vordergrund
gebracht wird.

Beim Laden einer Datei wird nun die Datei (klarerweise ;-) ) geladen, und danach wird die richtige View instanziert und im MainWindow angezeigt. Außerdem wird dieser View
das entsprechende ViewModel zugewiesen (View.DataContext)

- ViewModels Namespace -
Nomen est omen - in diesem Namespace sind alle ViewModels aufgehoben. Das "BaseL3dFileViewModel" ist die Basisklasse für alle ViewModels die direkt im MainWindow angezeigt werden.
Es implementiert einige Methode die vom MainWindow benutzt werden, hauptsächlich um Menü-Befehle vom MainWindow an die ViewModels "weiterzureichen"

- Views Namespace -
Beinhaltet Oberflächen und Helper dafür

- Settings Namespace -
Diverse Einstellungen. Derzeit nur die RegistrySettings die Zugriff auf Einstellungen aus der Registry überlauben

- LogicWrappers Namespace -
"Logik-Teile" des Loksims. Könnte auch "Sonstiges" genannt werden ;-)


*********************************
* LoksimEditShaderEffectLibrary *
*********************************
Dieses Projekt hat (derzeit, 18.12.2011) einen einzigen Zweck: Zwei Images können in WPF nicht mit Standardmethoden übereinander geblendet werden. Jedoch bietet WPF etwas viel
mächtigeres: Man kann Shader für die Graphikkarte schreiben um beliebige Effekte zu implementieren. Dieses Projekt implementiert einen Pixel-Shader um für die Vorschau
von Wetterdateien zwei Images übereinander zu blenden. 


