//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//

const char HTTP_DEFAULT[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no" />
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
    <title>Wettkampf Zeitnahme</title>
    {css_style}
  </head>
  <body>
    <div style='text-align:left;display:inline-block;min-width:260px;'>
    <div id='_uhrzeit' class='c'>{uhrzeit}</div>
    <hr/>
    <div> <table>
      {tableRows}
      </table>
     </div> 
      <hr />
      <div>
        <table>
          <tr>
            <td>
              <form onsubmit="return confirm('Bahn I ungültig?');" action="/" method="post">
              <button class='redbtn' name='btnBahn1Invalid' value='1' type='submit'>Bahn I ung&uuml;tig?</button></form>
            </td>
            <td>
              <form onsubmit="return confirm('Bahn II ungültig?');" action="/" method="post">
              <button class='redbtn' name='btnBahn2Invalid' value='1' type='submit'>Bahn II ung&uuml;tig?</button></form>
            </td>
          </tr>
        </table>
      </div>
      <hr />
      <div><form onsubmit="return confirm('Zeiten zurücksetzen?');" action="/" method="post">
      <button class='redbtn' name='btnReset' value='1' type='submit'>Zeit Reset</button></form></div>
      <hr />
      <div><form action="/" method="post">
      <button class='yellowbtn' name='btn5minCountdown' value='1' type='submit'>5 min. Countdown</button></form></div>
      <hr />
      <div>
      </div>
      <div><input class='lnkbtn' type='button' value='Uhrzeit stellen' onclick="window.location.href='/setTime'" /></div>
      <hr/>
       <div></div>
      <div><input class='lnkbtn' type='button' value='Download CSV' onclick="window.location.href='/?download&filename=/zeiten.csv'" /></div>
      {js}
    </div>
  </body>
</html>
)=====";


const char HTTP_SETTIME[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no" />
    <title>Wettkampf Zeitnahme</title>
    {css_style}
  </head>
  <body>
    <div style='text-align:left;display:inline-block;min-width:260px;'>
    <div id='_uhrzeit' class='c'>{uhrzeit}</div>
    <hr/>
     <div>
      <form action="/setTime" method="get">
      <div>
        <table>
          <tr>
           <td>Datum/Uhrzeit:</td><td align='left'>
           </tr>
           <tr>
            <td><input type='text' id='zeit' name='zeit' placeholder='TT.MM.JJJJ HH:MM:SS' value='' maxlength='20'></td>
          </tr>
        </table>
       </div>
       
       <div><button name='btnSave' value='1' type='submit'>Speichern</button></div>
       <div><input class='lnkbtn' type='button' value='Zur&uuml;ck' onclick="window.location.href='/'"/></div>
      </form>
      <div class='l c'>{sl}</div>
     </div>
     {js}
    </div>
  </body>
</html>
)=====";
