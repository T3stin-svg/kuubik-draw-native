# Kuubik Draw — native paperspace'i teostusplaan

Staatus: **JÄRGMINE ETAPP, mitte valmis funktsioon**. 2026-09-05.
Praegune viietunnine etapp muudab UI-d; alljärgnev lukustab järgneva tehnilise suuna.

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

Esmane tootmistee jääb libdxfrw. Täita senine tühi `RS_FilterDXFRW::addViewport`
ja asendada paper-space block'i dummy-käsitlus päris layout'i seostamisega.
LAYOUT, BLOCK_RECORD, paper-space entities ja VIEWPORT peavad säilitama owner-
seosed, eristatavad handle'id, nime ja viewport'i lipud. Eraldada lehe üldviewport
mudelit näitavatest viewport'idest; sama ID ei või korduda kõikidel vaadetel.

Enne write-back'i valideerida handle'ide unikaalsus ja kõik owner-seosed.
Mittetoetatud objekti vaikne kaotamine pole lubatud: säilitada läbipaistmatu
record/proxy, kui see on ohutult võimalik, vastasel juhul keelduda originaali
ülekirjutamisest koos konkreetse selgitusega. Save As koopiale jääb eraldi
teadlikult piiratud eksporditee. Selle toe puudumisel pole roundtrip sertifitseeritud.

## Tööjärjekord ja testid

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
