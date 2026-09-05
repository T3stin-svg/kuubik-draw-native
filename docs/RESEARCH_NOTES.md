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

### Kohaliku Windowsi seadistuste isolatsioon

`18d3f734` läbis puhta Windowsi CI 33964068198, kuid kohalik sõltumatu DXF-parser
leidis PLINE-ist kaks kattuvat punkti. `RS_Settings` kasutas explicit organization/
application konstruktorit, mis ignoreerib `setDefaultFormat(IniFormat)` valikut ja
loeb Windowsis NativeFormat-registrit. Seda käitumist kinnitab
[Qt 5.15 QSettings dokumentatsioon](https://doc.qt.io/archives/qt-5.15/qsettings.html#setDefaultFormat).

Parandus valib native-kihis formaadi selgelt, isoleerib ka SystemScope'i fallback'i
ja valikuinfo eraldi seadistused. GUI automaatika kasutab väljundkausta profiili;
CLI ja käsitsi arenduse vaatamiseks on absoluutne `KUUBIK_SETTINGS_DIR`. Tavaline
käivitus jääb sama registriprofiili juurde. Käivituse tõend kontrollib enne kirjutamist
ühist INI-faili ja seejärel mõlemasuunalist väärtuse lugemist. Testikomplekt võrdleb
registrit mälus enne/pärast; raport sisaldab vaid tõeväärtusi ja protsesside arvu.
PLINE-i producer kontrollib nüüd ka nullpikkusi/kattuvaid punkte, lisaks sõltumatule
ezdxf-kontrollile. Varasemate testikäivituste võimalikku registrimõju ei lähtestata
oletuslikult, sest kasutaja eelnevaid väärtusi ei ole salvestatud.

Paranduse `d35ec354` CI 33966232573 ja kohalik täiskordus läbisid; mõlemas tõendati
kümne protsessi isolatsioon ja muutumatu register. Täpne pakett on TEST_REPORT-is.

### Native eelvaate rangema kontrolli leiud — enne P0 parandusi

Alljärgnevad kaks leidu kehtivad `d35ec354` MSVC-artifakti kohta. Hilisem P0
parandussessioon lahendas mõlemad lähtekoodis ja kontrollis lokaalselt Qt/MinGW-ga;
MSVC CI on veel ootel. P0-B nullparanduste tõend piirdub uue puhta sünteetilise
ASCII modelspace-korpusega. Eraldi binary-DXF päise katse ebaõnnestus ja jääb
lahtiseks. Täpsed red/green-tulemused: [P0_CORRECTIONS](P0_CORRECTIONS.md).

Ka meie praegune libdxfrw väljund pole kogu objekti-struktuuri mõttes kadudeta:
`writePlotSettings` jätab owner 330 kirjutamata ja `writeObjects` ei loo
ACAD_PLOTSETTINGS dictionary't. Käsitsi ning CI-s salvestatud sünteetilisel DXF-il
annab ezdxf audit null viga, kuid ühe paranduse 202 (omanikuta PLOTSETTINGS kustutati
parseri mälus). Algne väike fixture sisaldab ka kahte table-handle parandust 110.
Geomeetria tagasilugemine läbib; nullparandustega faili ei väideta ega varjata
samal ajal Open CAD Studio auditivigu. See parandustee eelneb paperspace'i väravale.

Properties-i read-only dokumendikokkuvõte värskendub praegu selection/layer/MDI
callback'idega; LINE-i ja save'i järel võib entity/Modified näit jääda vanaks.
Vajalik on sama native dokumendi muutmisteavitus, mitte polling või teine mudel.

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
  Nüüd on olemas päris MSVC/Qt integratsioon: d35ec354 / CI 33966232573 ning
  kohalik native/independent kordus. Piirid ja tõend on TEST_REPORT-is.
- SARibbon ei paku CAD-i, paperspace'i ega automaatselt meie nelja otsese Draw-nupu
  kitsas aknas nähtavuse garantiid; need vastutused jäävad native-adapterisse.

## Tasuta failiteegid

| Komponent | Tõend ja kasutusotsus |
|---|---|
| libdxfrw | Olemasolev C++ GPLv2-or-later DXF-tee. Säilitame. Kuubiku `RS_FilterDXFRW::addViewport` on tühi; lisaks vajab teegi enda DXF VIEWPORT parser/writer välju 45/51/90 ning LAYOUT objektiteed. Täpne kaardistus PAPERSPACE_PLAN-is. |
| [ezdxf 1.4.4](https://github.com/mozman/ezdxf) | MIT/Python. Sõltumatu fixture-generaator ja DXF-testioraakel; juba CI-s. Ei ole DWG-mootor, ei kasutata ODA-konverteri wrapper'it. |
| [ACadSharp 3.7.1](https://github.com/DomCR/ACadSharp/tree/v3.7.1) | MIT/.NET. Tegelik DXF2018→DWG→DXF katse säilitas TEST-layout'i, A3, kaks viewport'i, skaalad, lukud, LINE/LWPOLYLINE/CIRCLE geomeetria ja paberiteksti; ezdxf audit 0 parandust. Kuid 25 VISUALSTYLE objekti kadus. Katse ei tõenda üldist kadudeta DWG-tuge. |
| [GNU LibreDWG](https://github.com/LibreDWG/libredwg) | GPLv3-or-later. Selle auditi ajal README kirjutajapiirangud ja GPLv2-integratsiooni küsimus; käitust ei testitud. Ei valita põhikirjutajaks. |

ACadSharp'i võimalik järgmine katse on eraldi protsessis failiadapter, mitte
teine CAD-dokument. Enne levitamist vajab see self-contained .NET-pakenduse,
kõigi sõltuvuste litsentsi, tundmatute objektide ja suurema failikorpuse kontrolle.
Selles UI-etapis uut .NET/Python runtime-sõltuvust tootesse ei lisata.

## Windowsi ehitus ja tõendi geomeetria

[Qt jom](https://wiki.qt.io/Jom) on paralleeltööd toetav nmake-asendus, mitte
toote runtime. CI kasutab fikseeritud 1.1.7 / kahe käsu paralleelsust ning
[avaldaja ZIP-i kontrollsummat](https://download.qt.io/official_releases/jom/jom_1_1_7.zip.mirrorlist).
Kasutaja rakendusse sellest uut sõltuvust ei lisata.

CI virtuaalse adapteri tegelikud režiimid leitakse Microsofti
[EnumDisplaySettings](https://learn.microsoft.com/windows/win32/api/winuser/nf-winuser-enumdisplaysettingsw)
abil. Suurimat oletatavat resolutsiooni ei eeldada. Qt5.15.2
[Windowsi geomeetriaadapter](https://github.com/qt/qtbase/blob/v5.15.2/src/plugins/platforms/windows/qwindowswindow.cpp)
annab lõpliku maximumHeight-piirangu native akna max-track suuruseks koos raamiga.
Automatiseeritud widget-katse kasutab seda 1080-pikslise kliendiala jaoks, et
töölaua vaikimisi max-track ei kärbiks seda 1061-ni. Tõendi pikslid ja native
geomeetria peavad endiselt vastama küsitud mõõdule; pilti ei skaleerita tagantjärele.

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

## Arenduse käigus leitud eraldi piirangud

- Windowsi `qoffscreen` diagnostikapildil puudusid lokaalselt UI fondiglüüfid ja
  fontide asendusmõõdud suurendasid paneele; sama binaari `qwindows` pilt sisaldab
  õiget teksti. Seetõttu ei kasutata offscreen-pilte visuaalse referentsi tõendina.
  Offscreen jääb native geomeetria/Undo regressiooni keskkonnaks; visuaalsed piirid
  ja 100/125/150% pildid kontrollitakse qwindows-iga.
- Päritud `console_dxf2png.cpp` liidab SVG `--outfile` argumendi alati sisend-DXF-i
  kausta külge. Absoluutne väljundtee andis lokaalses 6ef7f74e katses puuduva SVG,
  kuid protsess tagastas 0. Olemasolev CI kasutab samas kaustas ainult failinime ja
  kontrollib tegelikku SVG-d sõltumatult. Absoluutsete teede ning ebaõnnestunud
  ekspordi veakoodi parandus jääb eraldi failiekspordi ülesandeks; PDF-i CLI tee
  selles katses töötas. Seda viga ei peideta nõrgema failitesti taha.
