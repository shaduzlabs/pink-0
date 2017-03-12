var wsClient;

append = function(text)
{
  document.getElementById("events").insertAdjacentHTML('afterbegin', text);
}

currentTimestamp = function()
{
  var date = new Date();
  return '<div class="timestamp">' + date.toDateString() + " - " + date.toLocaleTimeString() +
    '</div>';
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

togglePlay = function()
{
  sendCommand("togglePlay")
}

createSocket = function()
{
  wsClient = new nmws.Client("127.0.0.1", "8888");

  wsClient.onConnectionEvent(function(connected_)
  {
    var date = new Date();
    sendCommand("getTempo");
    sendCommand("getLoopLength");
    sendCommand("getMultipliers");
    sendCommand("getMultiplierIndex");
    sendCommand("getNumberOfPeers");
    sendCommand("isEnabled");
    sendCommand("isPlaying");

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

        if(result.hasOwnProperty('tempo'))
        {
          document.getElementById("value-tempo").value = result['tempo'];
        }
        else if(result.hasOwnProperty('loopLength'))
        {
          document.getElementById("value-loopLength").value = result['loopLength'];
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
          document.getElementById("value-numPeers").value = result['numPeers'];
        }
        else if(result.hasOwnProperty('enabled'))
        {
          if(result['enabled'])
          {
            document.getElementById("led-enabled").classList.add("led-active");
          }
          else
          {
            document.getElementById("led-enabled").classList.remove("led-active");
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
        append('<div class="siimple-small siimple-alert siimple-alert--error">' + currentTimestamp() + ' malformed message received: ' + e + '</div>');
      }
  });

  wsClient.onError(function(errorCode_, errorDescription_)
  {
    append('<div class="siimple-small siimple-alert siimple-alert--error">' + currentTimestamp() +
      errorCode_ + ': ' + errorDescription_ + '</div>');
      setTimeout(function(){ createSocket(); }, 5000);
  })
}

window.onload = function()
{
  createSocket();
}
