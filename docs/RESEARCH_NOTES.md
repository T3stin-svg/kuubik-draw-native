# Kuubik Draw — auditi ja tasuta komponentide uuring

Kuupäev: 2026-09-05. See on sanitiseeritud koond, mitte kliendiandmete arhiiv.
Täielikud lokaalsed katsed asuvad ignoreeritud `.artifacts` kaustas. Neid ei
eeldata ehitamisel ega tavalise kasutaja arvutis.

## Põhiotsus

Jätkame LibreCAD v2.2.1.5 C++/Qt native-fork'i Windowsis. Ei vii toodet tagasi
Reacti, ei vaheta Open CAD Studio vastu ega lisa litsentsitasulist CAD SDK-d.
Tasuta failiteek ei lahenda dokumendimudelis puuduvaid layout'e/viewport'e.
Sama nimega nupp ei tõenda AutoCADiga sama kasutusjada: praegune COPY-test tõendab
kohapealset native duplikaati ja MOVE sisaldab LibreCADi dialoogi.

## Open CAD Studio — päriselt katsetatud

- [Projekt](https://github.com/HakanSeven12/OpenCADStudio), Windows portable v0.9.8,
  commit `1516694c72ae94ef7cf03750d4fbb070a8c81ab3`.
- Testitud EXE SHA-256:
  `db37867e80f3731210a5dfcdd02c9a96a47da2b0d993e465f8d8dd7f22497421`.
- Rust/iced/wgpu; GPL-3.0. Seda ei käsitleta Qt-komponendina ega kopeerita GPLv2
  fork'i ilma ühilduvust kontrollimata.
- Sünteetiline A3-leht, kaks 160×160 mm viewport'i, 1:50 ja 1:100. Algne paar
  valmistati ezdxf-iga; nullist kahe MVIEW loomist GUI-s ei tõendatud.
- GUI-s: vaadete lubamine, sõltumatu zoom, lukustuse kontroll, mõõtkava taastamine,
  viewport'i kaudu LINE lisamine, mõlema vaate uuenemine, salvestamine ja taasavamine.
- Native vektor-PDF: 5000 mm joon andis 100.000012 ja 50.000006 mm. PDF-is ei
  olnud rasterpilte. A3 MediaBox ümardus 420.158×297.039 mm; tekst oli piirjoontena.
- **Failitäpsus ebaõnnestus:** salvestatud DXF-il duplicate handle #1, 43
  parandustegevust, vigased owner-seosed ja TEST-layout'i identiteedikadu.
  Parandatud sisendiga kontrollis DXF→DXF 45 parandust, DXF→DWG→DXF 42 parandust.
  Teises variandis muutus viewport'i group 68 nulliks. Oma GUI suutis osa
  vigastest failidest siiski kuvada; see ei tähenda välise faili korrasolekut.
- Esialgse fixture'i puuduv viewport'i enabled-bit ei olnud programmi viga;
  korrigeeritud `reference-on.dxf` kontroll kordas struktuuriprobleeme.

Järeldus: hea töövoo näidis, mitte selle katse põhjal turvaline asendus meie mootorile.
AutoCADis tagasilugemist ja üldist DWG-ühilduvust ei kontrollitud.

## SARibbon — selle arendusetapi UI-komponent

- [v2.9.0](https://github.com/czyt1988/SARibbon/tree/v2.9.0), commit
  `806e3e93be4dd7676697d3017282a4359519e053`, MIT.
- Qt >=5.12, Core/Gui/Widgets/Svg; Qt5 puhul C++14 miinimum sobib meie Qt5.15/C++17-ga.
- Large/Medium/Small nupud saavad kasutada sama QAction-objekti.
- SARibbonBar sobib olemasoleva Kuubik-konteineri sisse; QWindowKit/frameless OFF.
- Säilitada native kiirklahvid, Kuubiku teksti/ikooni stabiilsus, toolbari ja
  kihivalija omand ning Classic-vaatesse taastamine.
- Varasem hinnang oli **lähtekoodipõhine, mitte kompileeritud integratsioon**.
  Selle etapi Windowsi tulemused lisatakse TEST_REPORT-i alles pärast kontrolli.
- SARibbon ei paku CAD-i, paperspace'i ega automaatselt meie nelja otsese Draw-nupu
  kitsas aknas nähtavuse garantiid; need vastutused jäävad native-adapterisse.

## Tasuta failiteegid

| Komponent | Tõend ja kasutusotsus |
|---|---|
| libdxfrw | Olemasolev C++ GPLv2-or-later DXF-tee. Säilitame. Kuubiku `RS_FilterDXFRW::addViewport` on tühi; teegi record-tugi ei tähenda rakenduse layout-tuge. |
| [ezdxf 1.4.4](https://github.com/mozman/ezdxf) | MIT/Python. Sõltumatu fixture-generaator ja DXF-testioraakel; juba CI-s. Ei ole DWG-mootor, ei kasutata ODA-konverteri wrapper'it. |
| [ACadSharp 3.7.1](https://github.com/DomCR/ACadSharp/tree/v3.7.1) | MIT/.NET. Tegelik DXF2018→DWG→DXF katse säilitas TEST-layout'i, A3, kaks viewport'i, skaalad, lukud, LINE/LWPOLYLINE/CIRCLE geomeetria ja paberiteksti; ezdxf audit 0 parandust. Kuid 25 VISUALSTYLE objekti kadus. Katse ei tõenda üldist kadudeta DWG-tuge. |
| [GNU LibreDWG](https://github.com/LibreDWG/libredwg) | GPLv3-or-later. Selle auditi ajal README kirjutajapiirangud ja GPLv2-integratsiooni küsimus; käitust ei testitud. Ei valita põhikirjutajaks. |

ACadSharp'i võimalik järgmine katse on eraldi protsessis failiadapter, mitte
teine CAD-dokument. Enne levitamist vajab see self-contained .NET-pakenduse,
kõigi sõltuvuste litsentsi, tundmatute objektide ja suurema failikorpuse kontrolle.
Selles UI-etapis uut .NET/Python runtime-sõltuvust tootesse ei lisata.

## Säilitatavad varasema projekti ideed

Vana veebirepo 133 nõuderida ja kuus visuaalolekut on nõuete allikad, mitte
native-toote teenitud skoorid. Taaskasutame layout'i identiteedi, ühise mudeli,
atomic Undo, sama kaameratransformatsiooni ja tundmatute objektide hoidmise nõudeid.
Home-paneelide mõõdud on talletatud DEVELOPMENT_PLAN-is. Referentspiltide pikslid
jäävad privaatseks; avaldada võib päritolu, mõõte ja oma programmi ekraanipilte.

## Lahtised päris toote piirangud

Paperspace, redigeeritav Properties-palett, täielik AutoCADi käsu-elutsükkel,
lai DWG/DWT/XREF failikorpus, taastumine ja suured joonised vajavad eraldi etappe.
Täpse visuaali väide vajab sama oleku, keele, DPI ja aknamõõdu võrdlust, mitte
ainult värvi või paneelipiiride testi. Eesmärgi täitmise protsenti ei oletata.
