# Kuubik Draw — SARibboni viietunnine arendusetapp

Staatus: **UI-etapp rakendatud ja kontrollitud; omaniku ülevaatus ootel**.
Algus 2026-09-05 10:00 UTC. Viietunnise akna ülevaatuse siht oli 15:00 UTC.
Vastutaja: üks integratsiooniomanik; Herdr ei ole selles keskkonnas saadaval.

Järgnev P0-parandussessioon algab `8a5f7ae0` pealt ja on eraldi allpool kirjeldatud
UI-etapist. P0-A üheksa Propertiesi olekut ning P0-B puhas ASCII modelspace-korpus
on lokaalselt Qt/MinGW build'iga kontrollitud. Mõlema MSVC CI ja omaniku kinnitus
on ootel. Täpne päevik: [P0_CORRECTIONS](P0_CORRECTIONS.md).
Selle sessiooni push/remote CI vajab Reio uut otsest luba.

## Kinnitatud eesmärk

Töötav native Windowsi eelvaade SARibboni ja AutoCAD 2024 Home-paigutusega.
LibreCADi QAction, dokumendid, geomeetria, valik, kihid ja Undo jäävad ainsaks
tõeallikaks. Paperspace on järgmine mootoriarenduse etapp, mitte selle sprindi
valmisfunktsioon. Kogu toote täpse AutoCADi vastavuse eesmärki ei vähendata.

- Haru: `codex/autocad-visual-integration-root`.
- Puhas lähtepunkt: `9968198bbe72165e28c48f8e37109fa3eb103212`.
- Kontrollitud Windowsi lähtebuild: [33919335101](https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33919335101).
- Selle ZIP-i SHA-256: `60bf9445fdc6206e8f1b21a392d72dece93714a56feabc435bf7cb66cca27550`.
- Kasutaja lubas tööharule push'i ja Windowsi CI. Merge, release, tagide muutmine,
  tasulised teenused ja kliendiandmete avaldamine ei ole lubatud.
- Qt/MSVC kohalikku arenduskeskkonda ei ole; ehituse autoriteet on Windowsi CI.

## Kohustuslik töö

- [x] Kontrollida puhast Git-olekut ja product-haru põlvnemist.
- [x] Salvestada plaan ja uuringute koond native-reposse.
- [x] Lisada SARibbon v2.9.0 fikseeritud lähtekood ja litsents qmake-build'i.
- [x] Asendada ribboni esitlus, säilitades olemasolevad QAction-id ja avaliku liidese.
- [x] Home: Draw, Modify, Annotation, Layers, Block, Properties, Groups, Utilities,
  Clipboard, View. Groups jääb ausalt mittetoetatuks, mitte teise käsu alias'eks.
- [x] Liita Layers/Layer; paigutada native pen-kontrollid Properties-paneeli.
- [x] Säilitada neli otsest suurt Draw-käsku 1200/1280 loogilise piksli laiuses.
- [x] Säilitada Application/Quick Access, kuus sakki, native Tool Options ja Classic.
- [x] Lisada tegeliku nähtavuse, paneeligeomeetria ja esitusolekute regressioonid.
- [x] Läbida Windowsi build, portable-smoke ja sõltumatu DXF/PDF/SVG tagasilugemine.
- [x] Viia käesolev plaan, seisund, otsused, teekaart ja testiraport kooskõlla.
- [x] Koostada kontrollitud EXE/ZIP, SHA-256, pildid, proovimisjuhend ja piirangud.

## Ajakava ja otsustusväravad

| Aeg algusest | Töö / värav |
|---|---|
| 0:00–0:25 | Lähtepunkt, dokumentatsioon, sõltuvuse kontroll |
| 0:25–1:20 | SARibboni adapter ja esimene CI |
| 1:20–3:00 | Paigutus, native kontrollid, teema ja testid |
| 3:00–4:00 | Täielik CI ja paperspace'i tehniline plaan |
| 4:00–5:00 | Paranduste puhver ja üleandmine |

Pärast kolmandat tundi uusi funktsioone ei lisata. Ebaõnnestunud build ei muutu
kontrollitud eelvaateks. Säilitatakse viimane roheline pakett ja täpne jätkupunkt;
teste ei nõrgendata ning kasutaja muudatusi ei tühistata.

## Kontrollitav visuaalleping

Etalon: AutoCAD 2024.1.2, EN, Drafting & Annotation, Dark, 1920×1080 / 100%.
Varasema veebiprojekti mõõdud on sisendandmed, mitte native PASS-tõend. Home'i
paneelide laiused on järjekorras 225, 250, 189, 273, 161, 262, 72, 97, 91, 53 px;
piirid mõõdetakse ribboni sisuala suhtes (mitte aknaraami ekraanikoordinaadist).
Tsoonipiiri sihttolerants on 2 loogilist pikslit; kitsas aken kasutab eraldi
kokkusurumisreegleid. Mitte ükski nende mõõtude test ei sertifitseeri kõiki
nuppe, töövooge ega kogu AutoCADi pariteeti.

Säilitada UI-contract schema 2 ja vanad väljad; lisada ribboni teostus/versioon,
paneelide ja kontrollide geomeetria ning tegeliku nähtavuse tulemused.
QAction peab olema sama objekt; peidetud nupu kunstlik klikk ei ole PASS.

## Edenemispäevik

- 2026-09-05 10:00 UTC: rakendamine algas. Tööpuu puhas. Product-haru remote-ref
  toodi kontrollimiseks alla; `origin/kuubik/visual-v0.2` on HEAD-i eellane.
- Uuema `9968198b` Windowsi tõendi tõttu on vanemate dokumentide d17e8b2-põhised
  "pole Windowsis kontrollitud" väited ajaloolised; need sünkroniseeritakse.
- 2026-09-05 10:17 UTC: esimene lähtekood `b694b5d8` push'itud lubatud tööharule;
  Windowsi CI [33960243802](https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33960243802)
  käivitatud. Lokaalne ikooni-, litsentsi-/SHA-, PowerShelli süntaksi-, whitespace-
  ja Gitleaks-kontroll läbisid. Ei ole veel kontrollitud Windowsi eelvaade.
- Uuendatud seisundi/teekaardi/otsuste/jätkamise dokumendid ning lisatud
  sõltumatu geomeetria- ja interaktsioonikontrolli negatiivsed testid.
- 2026-09-05 10:37 UTC: esimene MSVC/Qt build ja pakendamine läbisid, kuid UI
  käivitus ebaõnnestus `QAction::setVisible` juures. Põhjus tuvastatud upstream
  `SARibbonBar::showMinimumModeButton(false)` null-action'i kasutuses; Kuubik ei
  vaja seda vaikimisi puuduvat nuppu. Adapterist eemaldatud väljakutse, mitte
  muudetud vendori lähtekoodi. See run ei ole kontrollitud eelvaade.
- 2026-09-05 10:43 UTC: parandatud `6ef7f74e` lähtekoodil käivitati
  [teine CI](https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33961402868).
  Ehitamise ajal täpsustatud kogu esivanemaahela clipping'u kontrolli, eemaldatud
  reserveeritud SARibboni tiitlirida ja dubleerivad paneelivahed. Native Pen'i
  kolm valikut jäävad Properties-is nähtavale, viis lisakäsku saavad samade
  QAction-idega kompaktse esituse ning Classic taastab toolbar'i algse koosseisu.
  Lisatud viis sama stiili originaal-SVG-d (kokku 71 mapping'ut / 105 viidatud SVG-d).
- 2026-09-05 11:02 UTC: teine build/pakett käivitub, põhileping läbis. Uus
  referentskatse avastas Windowsi 1920×1061 clamp'i ning pilt näitas ka kärbitud
  ribbonit ja heledaid Fusion-nuppe. Tööala nõuet 1920×1080 ei vähendata; CI
  töölaud suurendatakse 2560×1440-le. Adapteri teegi-põhised suuruse/stiiili
  parandused ja tab'i hit-rect'i kontroll lähevad järgmisse ehitusse.
- 2026-09-05 11:28 UTC: `236b2b51` build/pakett läbis; CI virtuaalne adapter ei
  toeta 2560×1440 ja visual-smoke ei käivitunud. Kohalik qwindows näitab tumedat
  teemat, kuut toimivat sakki, Space/GRID roundtrip'i ning kõiki kolme native Pen
  valikut. Viie esimese Home-paneeli piirid on täpsed; Properties on 7 px liiga
  lai. Järgmine parandus kasutab tegelikult pakutavate Windowsi ekraanirežiimide
  loendit ning parandab ka kokkusurutud QWidgetAction'i enabled-peegelduse.
- Kohaliku GUI diagnostika algne COPY tõrge oli puuduv sünteetiline sisend-DXF;
  sama tõrge kordus baaspaketil. Õige fixture'iga läbisid nii 9968198b kui ka
  6ef7f74e LINE/PLINE/COPY/MOVE ja Undo/Redo. CI fixture'i ei muudetud.
- Lisatud arenduse AI-üleandmisarhiiv ilma release-tagi nõudeta. Arhiiv kontrollib
  ZIP-i räsi ja binaari manifesti ning võtab juhendid samast Git-snapshot'ist,
  mitte muudetud tööpuust. Sünteetilised positiivsed ja negatiivsed testid läbivad;
  päris üleandmine tehakse alles kontrollitud Windowsi paketist.
- CI [33963838013](https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33963838013)
  loetles adapteri režiimid: suurim 1920×1080, suurema kõrgusega vaid 1600×1200.
  Seetõttu annab automatiseeritud Qt-test oma kliendiala kõrguse piirangu ise,
  säilitades qwindows-i, native aknaraami ning nõutud 1920×1080 mõõtude range
  kontrolli. Salvestatakse eraldi puutumatu idle- ja testijärgne widget-pilt.
  Need pole Windowsi töölaua täielikud ekraanipildid ega OS-i DPI-seadete sertifikaat.
- Ehitust kiirendatakse Qt ametliku jom 1.1.7 kahe paralleelse käsuga; kontrollitud
  avaldaja SHA-256. MSVC, Qt5.15 ja qmake-lähteprojekt ei muutu ning jom ei lähe
  toote portable-paketti. Nmake jääb kohalikuks alternatiivseks build-käsuks.

- 2026-09-05: `18d3f734` läbis CI 33964068198 koos kõigi täpsete paneelipiiride,
  1920×1080 kliendiala, 100/125/150% qwindows-katsete ja sõltumatu failikontrolliga.
  Kohalik sõltumatu parser leidis siiski PLINE-i kattuvad punktid: native
  seadistuskiht luges testiprofiili asemel Windowsi registrit. Seda paketti ei
  valitud lõplikuks üleandmiseks. `d35ec354` isoleerib mõlemad seadistusliidesed,
  CLI ja valikuinfo ning lisab nullpikkuse ja registri muutumatuse kontrolli.
  [CI 33966232573](https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33966232573)
  käivitatud 12:30 UTC. Lõplikku staatust ei järeldata eelneva lähtekoodi PASS-ist.

Lõpptulemus: `d35ec35486912e4bca1fdc2a7125ecc6d53580eb`, CI 33966232573
läbis 12:44:15 UTC. ZIP 43,773,937 baiti, SHA-256
`4c3a2d8c2e36918e3fa77b0c106a174d1ea01bf27b6ecb0ea8d3400fa1c382d0`.
Kohalik täiskordus, sõltumatu DXF/PDF/SVG, qwindows-geomeetria ja kümne protsessi
seadistusisolatsioon läbisid; kõigi kümne Home-paneeli piirierinevus 0 px.
Windowsi sisendiga joonistatud LINE salvestus õigesti (62,5 mm).
Püsitõendid ja OWNER_REVIEW on repos; arenduse AI-arhiiv seob täpse binaari hilisema
dokumentatsiooni snapshot'iga ilma release'i loomata.

Paperspace jäi plaaniks. Järgmises parandusetapis tuleb lahendada rangema DXF-auditi
omanikuta PLOTSETTINGS-kirje ja Properties-i dokumendikokkuvõtte hilinenud refresh.
Need leiud on avalikult eristatud läbinud ribboni/geomeetria kontrollidest.
Omaniku lõplik visuaalne heakskiit, täielik AutoCAD-pariteet ning kogu faili
nullparandustega säilimine ei kuulu selle etapi PASS-väitesse.

## Ribboni komponendi- ja olekuleping

Disainisüsteemi kontroll suunab kasutama ühtset `kuubik-dark.qss` teemat ja
olemasolevat KuubikIconRegistry SVG-süsteemi, mitte uusi Autodesk-varasid.

| Olek / komponent | Nõue ja kontroll |
|---|---|
| Vaikimisi nupp | Kuubiku nimi/ikoon; sama native QAction, mitte koopiakäsk |
| Hover / vajutatud / checked | Teema eristatav taust; checked peegeldab native QAction'i |
| Klaviatuurifookus | StrongFocus; Space käivitab nähtava GRID-nupu ja taastab oleku |
| Disabled | Native enabled muutus jõuab ka SARibboni esitlus-action'isse |
| Native nime/ikooni muutus | LINE konstruktsioonivariant ei asenda ribboni püsivat nime/ikooni |
| Kokkusurutud paneel | Peitub ainult kohalik esitlus; menüü kasutab samu native käske |
| Puuduv funktsioon | Groups on disabled, selgitava tooltip'iga; ei alias'eeri Blocks'i |
| Classic | Native Pen ja Tool Options taastavad päritolu; Pen jälle horisontaalne |
| DPI | 100/125/150% qwindows renderdus; mõõdetud nähtavus ja geomeetria, mitte ainult suurusekonstandid |

Ikoon-only nuppudel säilivad ligipääsetav nimi ja tooltip. Kitsas aken ei peida
nelja põhigeomeetria käsku. Testi `ribbonInteraction` eristab hiirega tab'i valiku,
Space'i native QAction-triggerid ja olekute taastamise. Sõltumatu parser peab
ebaõnnestuma ka siis, kui üldine PASS-string on olemas, kuid alamkontroll ebaõnnestus.

## Seotud materjal

- [Uuringud](RESEARCH_NOTES.md)
- [Paperspace'i järgmine etapp](PAPERSPACE_PLAN.md)
- [Testiraport](TEST_REPORT.md)
