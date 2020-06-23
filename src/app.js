window.connection = new WebSocket('ws://' + location.hostname + ':81/');

//document.addEventListener("DOMContentLoaded", function() {
//  console.log("Connect to server get vars...")
//});

connection.onopen = function () {
  console.log("Connenting open!!")
  connection.send('Connect ' + new Date());
};

connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};

connection.onmessage = function (e) {
  console.log(e)
  var json_msg = JSON.parse(e.data);

  document.getElementsByClassName('average_score')[0].innerHTML = 0
  document.getElementsByClassName('games_played')[0].innerHTML = 0
  document.getElementsByClassName('percent_score')[0].innerHTML = json_msg["percent_score"];
  document.getElementsByClassName('score')[0].innerHTML = json_msg["score"];
  document.getElementsByClassName('target_score')[0].innerHTML = json_msg["target_score"];
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
        document.getElementsByClassName('percent_score')[0].innerHTML = 0
        document.getElementsByClassName('score')[0].innerHTML = 0
        document.getElementById('new_game').disabled = false;
      }
    };
    xhttp.open("GET", '/new', true);
    xhttp.send();
}