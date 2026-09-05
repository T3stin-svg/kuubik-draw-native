# Kuubik Draw — Reio ülevaatus

2026-09-05 native paperspace'i aluse arendusetapp. Source `81103460` läbis Windows
CI `33990304131` ning ZIP-i räsi/manifesti ja kohaliku täiskorduse kontrolli.
Täpne pakett ja räsi on [TEST_REPORT.md](TEST_REPORT.md) alguses. Tegemist on arenduse
eelvaatega, mitte uue avaliku release'i ega valmis AutoCAD-asendusega.

## Proovi umbes kümne minutiga

Kasuta uut sünteetilist joonist või koopiat, mitte kliendi originaali.

1. Vaata Home'i: Draw, Modify, Annotation, Layers, Block, Properties, Groups,
   Utilities, Clipboard, View. Täislaiuses on kümme paneeli; kitsamas aknas
   koonduvad osa käske menüüsse. Line, Polyline, Circle ja Arc peavad jääma otse
   nähtavaks. Groups on teadlikult välja lülitatud.
2. Vali kuus sakki ning kontrolli, et tekst ja ikoonid jääks loetavaks. Ava ka
   kokkusurutud paneeli menüü. See peab käivitama päris käsu.
3. Vali Layers-is aktiivne kiht, Properties-is värv, joone tüüp ja paksus.
   Joonista LINE ja PLINE, lõpeta Enteriga, seejärel Undo ja Redo. Katseta ka
   Escape'i. Propertiesi entity count ja Modified peavad kohe uuenema ka pärast
   salvestamist. Undo tõttu alles hoitud kustutatud objektid ei tohi loendurisse jääda.
   Kontrolli, et joonestamise native Tool Options oleks ülal nähtav.
4. Vaheta View → Workspace kaudu Kuubik → Classic → Kuubik. Klassikalised käsud,
   Pen ja Tool Options peavad säilima. See pole uue joonisemootori koopia.
5. Salvesta uue nimega DXF, sule ja ava uuesti. Ekspordi PDF ning võrdle nähtavat
   geomeetriat. Mõõtkavadega paperspace'i PDF ei kuulu veel sellesse eelvaatesse.
6. Ava sünteetiline layout-DXF. Praegune arendusbuild peab paperspace'i tuvastamisel
   keelduma Save/Save As'ist koos selgitusega; fail peab jääma muutumatuks.
   See katse kontrollib salvestuskaitset. Layout'i redigeerimine ja roundtrip ootavad.

Palun anna tagasisides konkreetne tegevusjada, oodatud/täheldatud tulemus ja
`build-manifest.json` lähtekoodi SHA. Kliendijoonist ega privaatset AutoCADi pilti
ei ole vaja avalikku GitHubi panna.

## Mis on valmis ja mis ootab järgmist etappi

SARibbon asendab native käskude esitluskihti. Ühtne tume teema, Kuubiku SVG-d,
Home'i paneelijaotus, native valikud, kitsas aken ja Classic on selle etapi sisu.
Kogu AutoCAD 2024 ekraani piksliline samasus ja kõigi käskude sama kasutusjada pole
veel tõendatud. Praegune COPY dubleerib native-jada kaudu samasse kohta ning MOVE
kasutab endiselt LibreCADi dialoogi.

Propertiesi entity/Modified viivitus ja omanikuta PLOTSETTINGS-kirje on parandatud.
Üheksa Propertiesi olekut ja määratletud modelspace'i failikorpus läbivad kontrolli.
Lisandunud native layout-metadata/Undo ja Qt kaamera alus on testitud, kuid pole
veel kasutajaliidesesse ühendatud. Binaarse DXF-i bool-parandus puudutab teeki;
native Save As kirjutab endiselt ASCII DXF-i. Tõendid ja piirid on TEST_REPORT-is.

Tõeline Model/Layout, kaks viewport'i, 1:50/1:100, lukud ning mõõtkavatäpne
vektor-PDF on [järgmise etapi plaan](PAPERSPACE_PLAN.md), mitte olemasolev funktsioon.
Tööharu arendusbuild kaitseb tuvastatud paperspace'i salvestamise eest, sest
native adapter ei säilita veel kogu struktuuri. Vana avalik `v0.2.0-preview.2`
release seda uut kaitset ei sisalda; kontrolli alati build-manifesti lähtekoodi.
DWG/DWT/XREF ei ole sertifitseeritud.

## Arendaja isoleeritud vaatamisprofiil

Uus lähtekood toetab keskkonnamuutujat `KUUBIK_SETTINGS_DIR`, mille väärtus peab
olema absoluutne kaustatee. See suunab Kuubiku ja native seadistused eraldi
INI-profiili, tavakasutaja Windowsi registriprofiili muutmata. Muutuja kehtib
ka `dxf2pdf` ja `dxf2svg` protsessidele. Tavaline käivitus ilma selle muutujata
kasutab endist Kuubiku profiili; seadistusi ei lähtestata ega migreerita.

Automaatne kontroll valib iga testijuhtumi jaoks oma profiili ning nõuab kümne
protsessi isolatsiooni ja muutumatut registrit. Korduskatse jaoks anna
`scripts/test-kuubik-portable.ps1`-le värske `RUNNER_TEMP`: varasemaid tõendeid
testiskript enam ei kustuta.
