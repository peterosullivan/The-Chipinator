String html_header = R"=====(
<!DOCTYPE HTML>
    <html lang='en'>
      <head>
        <meta charset='UTF-8'>
        <meta name='viewport' content='width=device-width, initial-scale=1, shrink-to-fit=no'>
        <link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css'>
        <script src="https://kit.fontawesome.com/5c486d6045.js" crossorigin="anonymous"></script>
        <title>The Chipinator</title>
        <style>
          html, body{
              height: 100%; background-color: #333; color: #fff; text-shadow: 0 .05rem .1rem rgba(0, 0, 0, .5);
          }
          .cover-container{
              max-width: 25em; margin: auto;
          }
          .card {
            position: relative;
            display: -ms-flexbox;
            display: flex;
            -ms-flex-direction: column;
            flex-direction: column;
            min-width: 0;
            word-wrap: break-word;
            background-color: #3c3c3c;
            background-clip: border-box;
            border: 1px solid rgba(0,0,0,.125);
            border-radius: .25rem;
            margin: 1.5rem 0 !important;
          }
          .shadow-sm {
            box-shadow: 0 .125rem .25rem rgba(0,0,0,.075) !important;
          }
          .card-body{
            padding: 1.25rem;
          }
          .card-title{
            font-size: 90px;
          }
        </style>
        <script>

          var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
          connection.onopen = function () {
            connection.send('Connect ' + new Date());
          };
          connection.onerror = function (error) {
            console.log('WebSocket Error ', error);
          };
          connection.onmessage = function (e) {
            console.log('Server: ', e.data);
            document.getElementsByClassName('score')[0].innerHTML = e.data;
          };
          connection.onclose = function () {
            console.log('WebSocket connection closed');
          };

          function newGame() {
            //connection.send("n");
            document.getElementById('new_game').disabled = true;
              var xhttp = new XMLHttpRequest();
              xhttp.responseType = 'json';
              xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                  var json = this.response;
                  document.getElementsByClassName('average_score')[0].innerHTML = json.average_score
                  document.getElementsByClassName('games_played')[0].innerHTML = json.games_played
                  document.getElementsByClassName('score_percent')[0].innerHTML = 0
                  document.getElementsByClassName('score')[0].innerHTML = 0
                  document.getElementById('new_game').disabled = false;
                }
              };
              xhttp.open("GET", '/new', true);
              xhttp.send();
          }

        </script>
      </head>
      <body class='text-center'>
        <div class='cover-container d-flex w-100 h-100 p-3 mx-auto flex-column'>
          <h1>The Chipinator</h1>
)=====";

String html_footer = R"=====(
    </div>
    </body>
    </html>
)=====";