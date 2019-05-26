//
// 2018-11-26 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//

const char HTTP_JS[] PROGMEM = R"=====(
<script>
  function Get(u){ 
    var h = new XMLHttpRequest(); 
    h.open('GET',u,false); 
    h.send(null); 
    return h.responseText; 
  } 

  function refreshState(rekursiv) { 
    var json_obj = JSON.parse(Get('/getValues')); 
    document.getElementById('_uhrzeit').innerHTML = json_obj.uhrzeit;

    var i;
    for (i in json_obj.ziele) {
     var element = document.getElementById('_ziel'+i);
     var statuselement = document.getElementById('_statusziel'+i);
     if (element != null) element.innerHTML = json_obj.ziele[i];
     if (statuselement != null) statuselement.innerHTML = json_obj.zielstatus[i];
    }

    if (rekursiv) setTimeout(function(){ refreshState(true); }, 1000); 
  } 
  
  /*init refresh:*/ 
  refreshState(true); 
</script>
)=====";
