<!doctype html>
<html>

<head>
  <title>pink web UI</title>

  <meta content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0' name='viewport' />

  <link rel="stylesheet" type="text/css" href="assets/css/siimple.min.css">
  <link rel="stylesheet" type="text/css" href="assets/css/dhtmlxslider.css">
  <link rel="stylesheet" type="text/css" href="assets/css/pink.css">

  <script src="assets/js/nmws.min.js"></script>
  <script src="assets/js/dhtmlxslider.js"></script>

  <script src="assets/js/scripts.js"></script>
</head>

<body>

  <div class="wrapper">
    
    <div class="siimple-h1"><div class="led" id="led-connected">&nbsp;</div>pink<span id="value-version">&nbsp;</span></div>

    <div class="content">

      <div class="toolbar">
        <div class="led" id="led-enabled" onClick="updateEnabled()">&nbsp;</div>
        <div class="siimple-label" id="value-numPeers">&nbsp;</div>
        <div class="led" id="led-playing" onClick="togglePlay()">&nbsp;</div>
      </div> <!-- toolbar -->

      <div class="control">
        <label class="siimple-label">Tempo: </label><input type="text" class="siimple-input" id="value-tempo" readonly></input>
        <div id="slider-tempo"></div>
      <div> <!-- control -->

      <div class="control">
        <label class="siimple-label">Length: </label><input type="text" class="siimple-input" id="value-loopLength" readonly></input>
        <div id="slider-loopLength"></div>
      </div> <!-- control -->

      <div class="control">
        <label class="siimple-label">Clock multiplier:</label>
        <select class="siimple-input" id="value-multipliers" onChange="updateMultiplier()"></select>
      </div> <!-- control -->

    </div> <!-- content -->

  </div> <!-- wrapper -->

  <input type="hidden" id="pink-ip-address" value="<?php echo $_SERVER['SERVER_ADDR']; ?>"></input>
</body>

</html>
