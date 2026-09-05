# Kuubik Draw — SARibboni viietunnine arendusetapp

Staatus: **TÖÖS**. Algus 2026-09-05 10:00 UTC. Ülevaatuse siht 15:00 UTC.
Vastutaja: üks integratsiooniomanik; Herdr ei ole selles keskkonnas saadaval.

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
- [ ] Lisada SARibbon v2.9.0 fikseeritud lähtekood ja litsents qmake-build'i.
- [ ] Asendada ribboni esitlus, säilitades olemasolevad QAction-id ja avaliku liidese.
- [ ] Home: Draw, Modify, Annotation, Layers, Block, Properties, Groups, Utilities,
  Clipboard, View. Groups jääb ausalt mittetoetatuks, mitte teise käsu alias'eks.
- [ ] Liita Layers/Layer; paigutada native pen-kontrollid Properties-paneeli.
- [ ] Säilitada neli otsest suurt Draw-käsku 1200/1280 loogilise piksli laiuses.
- [ ] Säilitada Application/Quick Access, kuus sakki, native Tool Options ja Classic.
- [ ] Lisada tegeliku nähtavuse, paneeligeomeetria ja esitusolekute regressioonid.
- [ ] Läbida Windowsi build, portable-smoke ja sõltumatu DXF/PDF/SVG tagasilugemine.
- [ ] Viia käesolev plaan, seisund, otsused, teekaart ja testiraport kooskõlla.
- [ ] Anda kontrollitud EXE/ZIP, SHA-256, pildid, proovimisjuhend ja piirangud.

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
