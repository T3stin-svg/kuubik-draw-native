# Kuubik Draw — native paperspace'i teostusplaan

Staatus: **DXF-aluse teostus käib; native paperspace pole valmis**. 2026-09-05.
Propertiesi ja PLOTSETTINGSi P0 parandused läbisid MSVC CI ning kohaliku portable-korduse.
P1-01a camera väljad ja P1-01b LAYOUT/BLOCK_RECORD lugemine on lokaalselt kontrollitud.
Kirjutuse ownership, native dokumendi integratsioon ja allolev kasutajavoog on ootel.
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
| `librecad/src/lib/filters/rs_filterdxfrw.h`, `addViewport` | Tühi callback; lisada päris native viewport'i import |
| `librecad/src/lib/filters/rs_filterdxfrw.cpp`, `addBlock` | `*Paper_Space` suunatakse dummyContainer'isse, mis impordi lõpus kustutatakse |
| `libraries/libdxfrw/src/drw_entities.cpp`, `DRW_Viewport::parseCode` | P1-01a lisas camera suuna/sihtpunkti/kõrguse, twist'i ja lipud; kohalik test läbitud |
| `libraries/libdxfrw/src/libdxfrw.cpp`, `writeViewport` | Camera väljad säilivad; täielik layout ownership on endiselt ootel |
| `libraries/libdxfrw/src/drw_interface.h`, `drw_objects.h` | P1-01b lisas LAYOUT/BLOCK_RECORD lugemise ja ühilduvad callback'id; native adapter neid veel ei kasuta |
| `libraries/libdxfrw/src/libdxfrw.cpp`, `writeEntity`, `writeObjects` | Uued handle'id, üldise owner'i kirjutamise ja ACAD_LAYOUT sõnastiku laiendamise vajadus |
| `librecad/src/lib/filters/rs_filterdxfrw.cpp`, `writeObjects` | Praegu ainult dokumendi üks PLOTSETTINGS; vajalikud layout-põhised lehesätted |

Lukustada esmalt minimaalne DXF 2018 record-leping: LAYOUT dictionary, layout'i
BLOCK_RECORD, paper-space entity owner, viewport'i identiteet/status, view-height,
twist ja lock lipud. Kõik arvväärtused tuleb initsialiseerida ka puuduvate DXF
väljade korral. Teegi DWG-lugejas olev samanimeline väli ei tõenda DXF-tuge.

Täita senine tühi `RS_FilterDXFRW::addViewport`
ja asendada paper-space block'i dummy-käsitlus päris layout'i seostamisega.
LAYOUT, BLOCK_RECORD, paper-space entities ja VIEWPORT peavad säilitama owner-
seosed, eristatavad handle'id, nime ja viewport'i lipud. Eraldada lehe üldviewport
mudelit näitavatest viewport'idest; sama ID ei või korduda kõikidel vaadetel.

Enne write-back'i valideerida handle'ide unikaalsus ja kõik owner-seosed.
Mittetoetatud objekti vaikne kaotamine pole lubatud: säilitada läbipaistmatu
record/proxy, kui see on ohutult võimalik, vastasel juhul keelduda originaali
ülekirjutamisest koos konkreetse selgitusega. Save As koopiale jääb eraldi
teadlikult piiratud eksporditee. Selle toe puudumisel pole roundtrip sertifitseeritud.

**Praegune SARibboni eelvaade ei rakenda seda kaitset. Layout'e sisaldavat
tootmisoriginaali ei tohi selle eelvaatega üle salvestada; katsetada koopiaga.**

## Native integratsioonipunktid ja eluea piirid

- `RS_Graphic` omab layout'ide registrit. Registri identiteet on stabiilne ID,
  mitte kasutaja muudetav nimi ega UI-saki indeks. `newDoc`, open-failure,
  document close ja save-as peavad registrit üheselt haldama.
- `RS_Document::removeUndoable` oskab praegu koristada ainult undone entity'id.
  Enne uute layout-muudatuste `RS_Undoable`-objektide lisamist tuleb laiendada
  omandit ja obsolete-redo koristust. Toores UI-pointer Undo payload'is ei sobi.
  Võimalik adapter hoiab dokumendi ID-sid ning enne/pärast väärtusseisu ja ühineb
  sama `startUndoCycle` / `endUndoCycle` tehinguga.
- `RS_GraphicView::toGui/toGraph` on praegu telgede skaalal/nihkel põhinev.
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
