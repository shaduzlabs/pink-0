var wsClient;
var linkEnabled;
var slider_tempo;
var slider_loopLength;

roundTwoDecimalDigits = function(value)
{
  return Math.round(value * 100) / 100;
}

sendCommand = function(command_, argument_)
{
  argument_ = typeof argument_ !== 'undefined' ? argument_ : 0;
  var cmd = new Object();
  cmd.command = command_;
  cmd.argument = argument_;
  var commands = Array();
  commands.push(cmd);
  wsClient.send(JSON.stringify(commands));
}

updateMultiplier = function()
{
  var multiplierIndex = document.getElementById('value-multipliers').value;
  sendCommand("setMultiplierIndex", parseInt(multiplierIndex))
}

updateTempo = function()
{
  var tempo = document.getElementById('value-tempo').value;
  sendCommand("setTempo", parseDouble(tempo))
}

updateLoopLength = function()
{
  var loopLength = document.getElementById('value-loopLength').value;
  sendCommand("setLoopLength", parseInt(loopLength))
}

updateEnabled = function()
{
  sendCommand("setEnabled", linkEnabled ? 0 : 1)
}

togglePlay = function()
{
  sendCommand("togglePlay")
}

initUI = function()
{
  sendCommand("getVersion");
  sendCommand("getTempo");
  sendCommand("getLoopLength");
  sendCommand("getMultipliers");
  sendCommand("getMultiplierIndex");
  sendCommand("getNumberOfPeers");
  sendCommand("isEnabled");
  sendCommand("isPlaying");
  slider_tempo.enable();
  slider_loopLength.enable();
}

resetUI = function()
{
  defaultTempo = 120;
  defaultLoopLength = 4;
  document.getElementById("led-connected").classList.remove("led-active");
  document.getElementById("led-enabled").classList.remove("led-active");
  document.getElementById("led-playing").classList.remove("led-active");
  document.getElementById("value-version").innerHTML = "&nbsp;";
  document.getElementById("value-tempo").value = "";
  document.getElementById("value-loopLength").value = "";
  document.getElementById("value-numPeers").innerHTML = "&nbsp";
  for(var i = document.getElementById("value-multipliers").options.length - 1 ; i >= 0 ; --i)
  {
      document.getElementById("value-multipliers").remove(i);
  }
  slider_tempo.setValue(defaultTempo);
  slider_tempo.disable();
  slider_loopLength.setValue(defaultLoopLength);
  slider_loopLength.disable();
  document.getElementById("led-playing").classList.add("led-disabled");
  document.getElementById("led-enabled").classList.add("led-disabled");
}

createSocket = function()
{
  var pinkIpAddress = document.getElementById("pink-ip-address").value;
  wsClient = new nmws.Client(pinkIpAddress, "8888");

  wsClient.onConnectionEvent(function(connected_)
  {
    var date = new Date();
    initUI();

    if(connected_)
    {
      document.getElementById("led-connected").classList.add("led-active");
    }
    else
    {
      document.getElementById("led-connected").classList.remove("led-active");
    }

  });

  wsClient.onMessage(function(data_)
  {
      try {
        result = JSON.parse(data_);

        if(result.hasOwnProperty('version'))
        {
          document.getElementById("value-version").innerHTML = result['version'];
        }
        if(result.hasOwnProperty('tempo'))
        {
          document.getElementById("value-tempo").classList.remove("temporary-value");
          document.getElementById("value-tempo").value = roundTwoDecimalDigits(result['tempo']) + " bpm";
        }
        else if(result.hasOwnProperty('loopLength'))
        {
          document.getElementById("value-loopLength").classList.remove("temporary-value");
          if(result['loopLength'] == 1)
          {
            document.getElementById("value-loopLength").value = roundTwoDecimalDigits(result['loopLength']) + " bar";
          }
          else
          {
            document.getElementById("value-loopLength").value = roundTwoDecimalDigits(result['loopLength']) + " bars";
          }
        }
        else if(result.hasOwnProperty('multipliers'))
        {
          var index = 0;
          var select = document.getElementById("value-multipliers");
          for(element in result["multipliers"])
          {
             var opt = document.createElement("option");
             opt.value= index;
             opt.innerHTML = result["multipliers"][index]; // whatever property it has
             select.appendChild(opt);
             ++index;
          }
        }
        else if(result.hasOwnProperty('multiplierIndex'))
        {
          document.getElementById("value-multipliers").selectedIndex = result["multiplierIndex"];
        }
        else if(result.hasOwnProperty('numPeers'))
        {
          if(result['numPeers']<1)
          {
            document.getElementById("value-numPeers").innerHTML = "&nbsp";
          }
          else
          {
            document.getElementById("value-numPeers").innerHTML = result['numPeers'];
          }
        }
        else if(result.hasOwnProperty('enabled'))
        {
          if(result['enabled'])
          {
            linkEnabled = true;
            document.getElementById("led-enabled").classList.add("led-active");
            document.getElementById("led-playing").classList.remove("led-disabled");
          }
          else
          {
            linkEnabled = false;
            document.getElementById("value-numPeers").innerHTML = '&nbsp;';
            document.getElementById("led-enabled").classList.remove("led-active");
            document.getElementById("led-playing").classList.add("led-disabled");
          }
        }
        else if(result.hasOwnProperty('playing'))
        {
          if(result['playing'])
          {
            document.getElementById("led-playing").classList.add("led-active");
          }
          else
          {
            document.getElementById("led-playing").classList.remove("led-active");
          }
        }
      }
      catch (e) {
        (console.error || console.log).call(console, e.stack || e);
      }
  });

  wsClient.onError(function(errorCode_, errorDescription_)
  {
    console.log("Websocket error #" + errorCode_ + ": " + errorDescription_);
    document.getElementById("led-connected").classList.remove("led-active");
    resetUI();
    setTimeout(function(){ createSocket(); }, 1000);
  });
}

initSliders = function()
{
  slider_tempo = new dhtmlXSlider({
    parent: "slider-tempo",
    size: 305,
    value: 120.0,
    step: 0.25,
    min: 20,
    max: 999
  });
  slider_tempo.attachEvent("onChange", function(value){
    document.getElementById("value-tempo").classList.add("temporary-value");
    document.getElementById("value-tempo").value = value + " bpm";
  });
  slider_tempo.attachEvent("onSlideEnd", function(value){
    sendCommand("setTempo", value);
  });

  slider_loopLength = new dhtmlXSlider({
    parent: "slider-loopLength",
    size: 305,
    value: 4.0,
    step: 0.25,
    min: 0.25,
    max: 32
  });
  slider_loopLength.attachEvent("onChange", function(value){
    document.getElementById("value-loopLength").classList.add("temporary-value");
    if(value == 1)
    {
      document.getElementById("value-loopLength").value = value + " bar";
    }
    else
    {
      document.getElementById("value-loopLength").value = value + " bars";
    }
  });
  slider_loopLength.attachEvent("onSlideEnd", function(value){
    sendCommand("setLoopLength", value);
  });
}

deinitSliders = function()
{
  if (slider_tempo != null)
  {
    slider_tempo.unload();
    slider_tempo = null;
  }

  if (slider_loopLength != null)
  {
    slider_loopLength.unload();
    slider_loopLength = null;
  }
}

window.onload = function()
{
  initSliders();
  resetUI();
  createSocket();

}

window.onunload = function()
{
  deinitSliders();
}
