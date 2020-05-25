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
              max-width: 42em; margin: auto;
          }
        </style>
        <script>
          function newGame() {
            var xhttp = new XMLHttpRequest();
            xhttp.responseType = 'json';
            xhttp.onreadystatechange = function() {
              if (this.readyState == 4 && this.status == 200) {
                var json = this.response;
                document.getElementsByClassName('average_score')[0].innerHTML = json.average_score
                document.getElementsByClassName('games_played')[0].innerHTML = json.games_played
                document.getElementsByClassName('score_percent')[0].innerHTML = 0
                document.getElementsByClassName('score')[0].innerHTML = 0
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