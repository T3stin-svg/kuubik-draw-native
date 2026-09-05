# Kuubik Draw — native paperspace'i teostusplaan

Staatus: **DXF-aluse teostus käib; native paperspace pole valmis**. 2026-09-05.
Propertiesi ja PLOTSETTINGSi P0 parandused läbisid MSVC CI ning kohaliku portable-korduse.
P1-01a camera väljad ja P1-01b LAYOUT/BLOCK_RECORD lugemine läbisid kohaliku ja MSVC kontrolli.
P1-01c piiratud kirjutus säilitab tunnused/seosed ja läbib kohaliku raw/0-0 auditi;
native salvestuskaitse, metadata/Undo ja Qt kaamera alus on kontrollitud.
DXF-adapteri ühendamine, paper entities ning allolev kasutajavoog on ootel.
Jooksev seis ja tõendid: [ROADMAP](ROADMAP.md), [TEST_REPORT](TEST_REPORT.md).

## Vastuvõetav vertikaalne läbilõige

Üks RS_Graphic-dokument, jagatud model space ja nimega A3 landscape layout TEST.
Lehel kaks sama mudeli 160×160 mm vaadet: 1:50 ja 1:100. Mõõtühik mm.
Topeltklõps siseneb vaate kaudu mudelisse; paberil väljaspool vaadet väljub.
Lukustatud vaates ei muuda rattazoom mudeli kaamerat ega mõõtkava.
Ühe viewport'i kaudu lisatud/muudetud LINE on kohe näha mõlemas.
Undo/Redo, DXF sulgemine/taasavamine ja sama lehe vektor-PDF on kohustuslikud.

## Üks dokument, kolm konteksti

- Layout'id kuuluvad olemasolevale native-dokumendile, mitte eraldi UI JSON-ile.
  Model space'i entity-id ei dubleerita iga viewport'i jaoks.
- Layout sisaldab paberühikutes entities-konteinerit, stabiilset identiteeti,
  nime, järjekorda, lehesätteid ja viewport-objekte.
- Viewport sisaldab paberiruumi raami, model-center'it, view-height'i, nurka,
  enabled/locked olekut ja seotud layout'i identiteeti.
- Aktiivne vaatekontekst on Model, Paper või ModelThroughViewport. Valik,
  snap, käsusisend ja koordinaadid saavad konteksti samast kontrollerist.
- Mudeli muutmine läbib olemasolevaid native-käske. Layout'i/viewport'i
  muudatused laiendavad sama dokumendi Undo/Redo ajalugu; eraldi UI ajalugu ei teki.

## Renderdus, koordinaadid ja plot

Mõõtkava on paper-height / model-view-height; 160 mm raamis tähendab 1:50
view-height'i 8000 mm ja 1:100 väärtust 16000 mm. Teisendus kasutab viewport'i
keskpunkti ja pöördenurka, mille ümber mudel paigutatakse. Renderdus, inverse
hit-test, snap ja PDF kasutavad sama transformatsiooni ning raamiga klippimist.
Mudel on endiselt topelt-täpsusega native geomeetria; ekraanipiksleid ei salvestata.

Paper-kontekstis zoomitakse lehte. Avatud ModelThroughViewport-kontekstis
muudab zoom ainult selle viewport'i kaamerat. Locked-olekus kaamera ei muutu;
mudelobjektide redigeerimine on endiselt lubatud. Vaate mõõtkava muutus ei
muuda model-entity koordinaate. PDF kasutab lehte 1:1, mitte fit-to-page'i.

## DXF ja andmete säilimine

Esmane tootmistee jääb libdxfrw. **Sellest ei piisa, et täita üks callback.**
2026-09-05 native-lähtekoodi järelkontroll leidis ka teegitaseme puudujäägid:

| Päris lähtekoodikoht | Praegune piirang / järgmine vajalik muudatus |
|---|---|
| `librecad/src/lib/filters/rs_filterdxfrw.h`, `addViewport` | Märgib salvestuskaitse; päris native viewport'i omand on ootel |
| `librecad/src/lib/filters/rs_filterdxfrw.cpp`, `addBlock` | `*Paper_Space` suunatakse dummyContainer'isse, mis impordi lõpus kustutatakse |
| `libraries/libdxfrw/src/drw_entities.cpp`, `DRW_Viewport::parseCode` | P1-01a lisas camera suuna/sihtpunkti/kõrguse, twist'i ja lipud; kohalik test läbitud |
| `libraries/libdxfrw/src/libdxfrw.cpp`, `writeViewport` | Camera väljad säilivad; piiratud layout'i kirjutus on lokaalselt kontrollitud |
| `libraries/libdxfrw/src/drw_interface.h`, `drw_objects.h` | P1-01b lisas LAYOUT/BLOCK_RECORD lugemise; native adapter kasutab LAYOUT-i esialgu salvestuskaitseks |
| `libraries/libdxfrw/src/libdxfrw.cpp`, `writeLayoutDocument` | Piiratud typed-eksport säilitab ID-d/owner'id ning kirjutab ACAD_LAYOUT; native adapteri ühendamine ja toetamatu sisendi värav on ootel |
| `librecad/src/lib/filters/rs_filterdxfrw.cpp`, `writeObjects` | Praegu ainult dokumendi üks PLOTSETTINGS; vajalikud layout-põhised lehesätted |

Lukustada esmalt minimaalne DXF 2018 record-leping: LAYOUT dictionary, layout'i
BLOCK_RECORD, paper-space entity owner, viewport'i identiteet/status, view-height,
twist ja lock lipud. Kõik arvväärtused tuleb initsialiseerida ka puuduvate DXF
väljade korral. Teegi DWG-lugejas olev samanimeline väli ei tõenda DXF-tuge.

Ühendada praegu salvestuskaitset märkiv `RS_FilterDXFRW::addViewport`
ja asendada paper-space block'i dummy-käsitlus päris layout'i seostamisega.
LAYOUT, BLOCK_RECORD, paper-space entities ja VIEWPORT peavad säilitama owner-
seosed, eristatavad handle'id, nime ja viewport'i lipud. Eraldada lehe üldviewport
mudelit näitavatest viewport'idest; sama ID ei või korduda kõikidel vaadetel.

Enne write-back'i valideerida handle'ide unikaalsus ja kõik owner-seosed.
Mittetoetatud objekti vaikne kaotamine pole lubatud: säilitada läbipaistmatu
record/proxy, kui see on ohutult võimalik, vastasel juhul keelduda originaali
ülekirjutamisest koos konkreetse selgitusega. Tulevane piiratud Save As koopiale
vajab eraldi säilivuse tõendit. Praegu keelab kaitse ka Save As'i; roundtrip pole sertifitseeritud.

**Varasem SARibboni arenduspakett ei rakenda seda kaitset. Tööharu P1-02a lisab
native Save/Save As/autosave/eksportimise keelu tuvastatud paperspace'ile;
source `fcad4372` läbis MSVC CI `33986140500`. See ei ole veel paperspace'i muutmise ega roundtrip'i tugi.**

## Native integratsioonipunktid ja eluea piirid

- `RS_Graphic` omab layout'ide registrit. Registri identiteet on stabiilne ID,
  mitte kasutaja muudetav nimi ega UI-saki indeks. `newDoc`, open-failure,
  document close ja save-as peavad registrit üheselt haldama.
- P1-02b teostab native väärtusregistri ja ühe metadata-snapshot'i olemasoleva
  Undo-tsükli kohta. Edit'i `id=0` loob uue tunnuse; olemasolev tunnus peab kuuluma
  elusale sama tüübi objektile. Import aktsepteerib lähtetunnuseid ainult tühja
  registri/ajalooga. Lugeja-adapteri ühendamine ning paper-entity konteinerid on veel ootel.
- `RS_Document::removeUndoable` koristab jätkuvalt borrowed entity'id. P1-02b
  lisab metadata omandi tsüklisse endasse; obsolete-redo vabastab payload'id
  koos tsükliga. Korduvad muudatused koonduvad ühte snapshot'i sama native
  `startUndoCycle` / `endUndoCycle` tehingu sees. Payload ei hoia UI-pointer'it.
- `RS_GraphicView::toGui/toGraph` on praegu telgede skaalal/nihkel põhinev.
  P1-03a lisab RS_PaperViewport'i Qt kaamera: WCS → Y-üles paber mm,
  pööre radiaanides, kehtiv raam ja 1e-6 mm arvulise täpsuse värav.
  Native registri valideerimine kasutab sama teisendust. Qt raster-/PDF-proov
  mõõdab geomeetriat ja mõlemat clip'i; päris renderdus/plot pole veel ühendatud.
  Pööratud viewport ei valmi ainult QPainter.rotate abil: olemasolevad käsud ja
  snap kasutavad samas klassis scalar toGuiX/toGuiY/toGraphX/toGraphY teisendusi.
  Kõik need tarbijad tuleb kaardistada; uus ühtne 2D transform peab teenindama
  vektorpunkte ja distantse õigesti, ilma mudelkoordinaate ümber kirjutamata.
- `RS_GraphicView::drawEntity` ja `RS_PainterQt` jäävad native renderdusteeks.
  Iga viewport'i renderdus salvestab/taastab painter'i klipi ja transformi.
  Valikuhandle'id, snap-markerid, joonetüübi skaala ja lineweight ei tohi pärida
  kogemata eelmise viewport'i olekut. Mitteplotitav viewport'i raam on eraldi
  paberobjekti omadus, mitte mudeli layer'i globaalne peitmine.
- Print Preview ja PDF peavad saama sama layout-konteksti. Ekraanil nähtud
  rasterpildi asetamine PDF-i ei ole vektor-PDF värava täitmine.
- Aktiivse viewport'i kustutamine vahetab konteksti ohutult Paper'iks, katkestab
  pooliku käsu ilma geomeetriat salvestamata ja puhastab valiku. Undo taastab
  identiteedi; aktiivse UI-fookuse taastamise poliitika testitakse eraldi.

## Tööjärjekord ja testid

### Järgmine kontrollpunkt — P1-02c valideeritud native import

2026-09-05 värske lähtekoodiülevaatuse tulemus. Teostada esmalt piiratud impordi
seostamine ning ajutine ekspordiesitus; Save-kaitse jääb alles kuni järgneva
salvestus/close/reopen värava läbimiseni. Faili lugemine muudab praegu native
geomeetriat jooksvalt, mistõttu metadata etapiviisiline kogumine üksi ei tõenda
kogu impordi atomaarset käitumist.

| Olemasolev koht | Minimaalne vajalik ühendus |
|---|---|
| `RS_FilterDXFRW::fileImport`, `addLayout`, `addViewport` | Koguda kirjed ajutiselt; kinnitada native metadata alles eduka terviklugemise ja allikavärava järel. DRW väärtused kopeerida konstruktoriga, mitte vigase shallow assignment'iga. |
| `dxfRW::processObjects` | Praegu vahele jäetav DICTIONARY peab andma tõendi tegeliku ACAD_LAYOUT liikmesuse kohta. LAYOUT-i owner üksi ei piisa. |
| `RS_FilterDXFRW::setEntityAttributes` | Siduda lähteline LINE-handle/owner päris native üksusega. `RS_Entity::getId()` on protsessisisene tunnus, mitte DXF-handle. |
| `RS_Graphic::commitImportedPaperSpace` | Kasutada olemasolevat tühja registri/Undo importteed; seoste eluiga peab lõppema koos dokumendiga. |
| `dxfRW::writeLayoutDocument` | Koostada ajutised kirjed elusast native geomeetriast ja metadatast. Püsivat teist geomeetriamudelit ei lisata. |

Native metadata vajab veel Model-layout'i, ACAD_LAYOUT dictionary ja mõlema
BLOCK_RECORD-i identiteeti ning native LINE-ide lähteseoseid. Viewport-handle
ja group 69 number on eri tunnused; kogu paberit kirjeldav nr 1 pole kolmas
mudelivaateaken. Uute tunnuste reserveerimine peab arvestama kõiki säilitatavaid
kirjeid. Praegune writer genereerib süsteemitabelite, juurdictionary ning
BLOCK/ENDBLK tunnused uuesti; kõigi DXF-handle'ide säilimist ei väideta.

Kaamera lähtepiir: grupid 12/22 on DCS-is, siht 17/27/37 WCS-is
([Autodesk VIEWPORT](https://help.autodesk.com/cloudhelp/2015/ENU/AutoCAD-DXF/files/GUID-2602B0FB-02E4-4B9A-B03C-B1D904753D34.htm)).
Piiratud +Z pealtvaate [ezdxf 1.4.4 teisendusest](https://raw.githubusercontent.com/mozman/ezdxf/v1.4.4/src/ezdxf/entities/viewport.py)
järeldub praeguse native tehase jaoks allolev kandidaat. See vajab geomeetrilist
tõendit enne adapterisse kinnistamist; raw-väljade võrdsusest ei piisa.

```text
nativeTwist = -rawTwist
nativeCenter = target + R(-rawTwist) * dcsCenter
export: dcsCenter = R(rawTwist) * (nativeCenter - target)
```

Eraldi planeerimiskatse võrdles keskpunkti ja mõlemat 5000-ühikulist baastelge
ezdxf 1.4.4 maatriksiga: 0/+30/−30°, mõlemad mõõtkavad, mitte-null siht/kese
ja ekspordi pöördvalem läbivad. See ei käivitanud veel native DXF-adapterit.

Säilitada WCS-siht ankruna, mitte teine iseseisvalt muudetav DCS-kaamera.
Allikavärav kontrollib samu baite, mida imporditakse: ASCII AC1032/mm,
Model+TEST, vastastikused layout/ploki seosed, dictionary-liikmesus, unikaalsed
tunnused ning ainult määratletud LINE/ristkülikulise VIEWPORT-i sisu. Esimene
etapp piirdub mudelruumi LINE-idega; paberigeomeetria vajab veel native omandit.
Tundmatud kirjed, külmutatud viewport-kihid, clipping-kontuurid, perspektiiv,
mittetasapinnaline kaamera ja säilitamata väljad ei saa toetatud staatust.
Arvestada subclass'i ning pesastatud 102 ulatust; typed-writer ei näe juba
lugejas kadunud andmeid.

Vastuvõtt viie kontrolliga:

1. A3 TEST import: kaks mudelivaateakent ja eraldi nr 1, täpsed tunnused/owner'id,
   tühi Undo ning Modified=false.
2. Mitte-null DCS-kese ja WCS-siht, 0/+30/−30°, teadaolevad kesk-/baaspunktid,
   1:50 ja 1:100 ning sõltumatud lukud; võrrelda tegelikku paberigeomeetriat.
3. Puuduv dictionary, vale owner/backlink, 102 võltsseos, külmutatud kiht ja
   tundmatu objekt ei läbi toetatud impordi väravat.
4. Hõredad/kõrged tunnused, kokkupõrked, Undo/Redo, import-failure ja newDoc
   ei jäta vanu seoseid ega võimalda tunnuste taaskasutust.
5. Järgnev salvestusvärav: native import → ekspordiesitus → save → dokumendi
   hävitamine → uus import, kaks korda; raw ID/owner/viewport-numbrid enne
   normaliseerimist, sama kaamerageomeetria ning sõltumatu audit 0/0.

Enne päris Save lubamist kontrollida hetke native sisu ja sihtvormingut uuesti,
enne backup'i või Save As'i olekumuutusi. Impordiaegne toetatud staatus ei kata
hiljem lisatud eksportijale toetamata üksusi.

### Kogu native töövoo järjekord

0. libdxfrw minimaalne record-leping ja import/export korpus: tõendada, et vajalikud
   andmed üldse läbivad teegi. Alles siis siduda UI nupud uute objektidega.
1. Native layout/viewport omand, eluiga ja Undo; mudeli külge sidumise testid.
2. Üks ühine transformatsioon: 0° ja 30°, pöördteisendus, klippimine, ühikud.
3. DXF sünteetiline korpus enne UI lubamist; import/export/import ning ezdxf audit.
4. Model/Layout sakid, viewport'i aktiveerimine, mõõtkava ja lukk; GUI-smoke.
5. Viewport'i kaudu LINE, undo/redo, mõlema vaate identne mudeliseos.
6. A3 vektor-PDF; 5000 mm LINE pikkused 100 ja 50 mm, tolerants 0.05 mm.
7. Layout'i koopia: sõltumatud paberobjektid/viewport'id, jagatud mudel;
   sulgemine/taasavamine säilitab kogu struktuuri.

Vajalikud negatiivsed testid: null/negatiivne mõõtkava, tühi mudel, kattuvad
viewport'id, lukustatud rattazoom, peidetud vaade, aktiivse vaate kustutamine,
vigane owner-handle, tundmatu entity, salvestustõrge, Undo pärast kontekstivahetust.
Automaatkontroll ei asenda AutoCAD 2024-s sama DXF-i avamist; see värav jääb eraldi.

## Jätku piir

Esimene läbilõige ei sisalda DWG sertifitseerimist, XREF-i, annotative scale'i,
mittetäisnurkset viewport'i clipping'ut, 3D-kaamerat ega uut PDF runtime-teeki.
Need ei tohi hiilida sisse enne põhilise dokumendi/transformatsiooni/roundtrip'i
kontrolli. SARibbon pole ühegi selle etapi mootorifunktsiooni asendus.
