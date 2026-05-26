# PlankaC: Eine quellenorientierte Implementierung einer Plankalkuel-nahen Notation

PlankaC ist ein C-basiertes Werkzeug fuer ein ausfuehrbares
Plankalkuel-Profil. Das Projekt liest `.plk`-Quellen, baut daraus eine
Prozedurtabelle, fuehrt Prozeduren aus und kann Compiler-Artefakte erzeugen:
Bytecode, typed IR, generiertes C, generiertes x86-64-ASM und 8086/DOS-nahes
ASM.

Der Anspruch ist bewusst technisch und nachpruefbar. PlankaC nennt sich nicht
eine vollstaendige Rekonstruktion des historischen Plankalkuels. Der aktuelle
Stand ist eine substanzielle ausfuehrbare Profil-Implementierung in C, mit
dokumentierten Erweiterungen und Tests.

## Bezug zu Zuse

Konrad Zuses Plankalkuel ist ein wichtiger frueher Entwurf fuer eine
hochsprachliche Formulierung von Rechenplaenen, Werten, Strukturen und
schematischen Aufgaben. PlankaC nimmt diesen Entwurf ernst: Prozeduren,
Wertbaenke, Typmarker, strukturierte Werte, Listen, Mengen, Relationen und
Schachmodelle sind nicht nur beschrieben, sondern als ausfuehrbare Quellen im
Repository abgelegt.

Die wichtigste moderne Vergleichsarbeit ist die Plankalkuel-Implementierung
von Rojas, Goektaktik, Friedland, Kroeger, Langmack und Kuehnel aus dem Jahr
2000. PlankaC nutzt diese Arbeit nicht als Marketingbehauptung, sondern als
Orientierung fuer Notation, Implementierungsfragen und Abgrenzung.

## Was laeuft

```text
build.bat
build\plankac.exe check
build\plankac.exe run calculator_demo
build\plankac.exe compile build\plankac_pipeline
build\plankac.exe native-c build\plankac_native_c
build\plankac.exe native-asm build\plankac_native_asm
build\plankac.exe asm8086 build\plankac_8086.asm
build\plankac_conformance.exe
```

Auf Linux wird derselbe Kern ueber `make ci` gebaut und geprueft.

## Wofuer das Projekt nuetzlich ist

PlankaC ist nuetzlich, wenn man eine alte Sprachidee nicht nur zitieren,
sondern ausfuehren, testen und in moderne Build-Prozesse einbinden will. Man
kann `.plk`-Dateien schreiben, Prozeduren pruefen, Bytecode exportieren, C
oder ASM erzeugen und die Laufzeit aus C heraus einbetten.

Das Projekt ist damit weder ein reines Dokument noch nur ein Taschenrechner.
Der Rechner ist eine sichtbare Oberflaeche. Der eigentliche Kern ist die
ausfuehrbare Sprach- und Compiler-Schicht.

## Grenzen

Nicht alles wird als historischer Kern ausgegeben. Die 3D-Prozeduren und
Grafikprofile sind PlankaC-Erweiterungen. Win16 und DOS sind eigenstaendige
Plattform-Builds. Die Release-Artefakte trennen moderne CI-Binaries von
experimentellen Plattform-Builds.

Die Dokumentation ist deshalb absichtlich genau: eine substanzielle
ausfuehrbare Plankalkuel-Profil-Implementierung in C, nicht die Behauptung
einer vollstaendigen historischen Ausgabe.
