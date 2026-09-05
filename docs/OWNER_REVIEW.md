# Kuubik Draw — Reio ülevaatus

2026-09-05 SARibboni UI-etapp. Täpne kontrollitud lähtekood, Windowsi build ja ZIP-i
räsi asuvad [TEST_REPORT.md](TEST_REPORT.md) alguses. Tegemist on arenduse
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
   Escape'i. Kontrolli, et joonestamise native Tool Options oleks ülal nähtav.
4. Vaheta View → Workspace kaudu Kuubik → Classic → Kuubik. Klassikalised käsud,
   Pen ja Tool Options peavad säilima. See pole uue joonisemootori koopia.
5. Salvesta uue nimega DXF, sule ja ava uuesti. Ekspordi PDF ning võrdle nähtavat
   geomeetriat. Mõõtkavadega paperspace'i PDF ei kuulu veel sellesse eelvaatesse.

Palun anna tagasisides konkreetne tegevusjada, oodatud/täheldatud tulemus ja
`build-manifest.json` lähtekoodi SHA. Kliendijoonist ega privaatset AutoCADi pilti
ei ole vaja avalikku GitHubi panna.

## Mis on valmis ja mis ootab järgmist etappi

SARibbon asendab native käskude esitluskihti. Ühtne tume teema, Kuubiku SVG-d,
Home'i paneelijaotus, native valikud, kitsas aken ja Classic on selle etapi sisu.
Kogu AutoCAD 2024 ekraani piksliline samasus ja kõigi käskude sama kasutusjada pole
veel tõendatud. Praegune COPY dubleerib native-jada kaudu samasse kohta ning MOVE
kasutab endiselt LibreCADi dialoogi.

Teadaolevad järgmised parandused: Properties-i entity/Modified kokkuvõte võib
pärast joonistamist või salvestust hilineda; see ei tähenda kadunud geomeetriat.
Rangem DXF-audit leiab ühe omanikuta PLOTSETTINGS-kirje, mistõttu nullparandustega
kogu faili säilimist veel ei lubata. Täpne leid on TEST_REPORT-is.

Tõeline Model/Layout, kaks viewport'i, 1:50/1:100, lukud ning mõõtkavatäpne
vektor-PDF on [järgmise etapi plaan](PAPERSPACE_PLAN.md), mitte olemasolev funktsioon.
**Ära kirjuta selle eelvaatega üle layout'e sisaldavat tootmis-DXF-i:** pärandadapter
ei säilita veel paperspace'i andmeid. DWG/DWT/XREF ei ole sertifitseeritud.

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
