var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  // Construct URL
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' + 
      pos.coords.latitude + '&lon=' + pos.coords.longitude;
  //var url = 'http://api.wunderground.com/api/***REMOVED***/conditions/q/' + pos.coords.latitude + ',' + pos.coords.longitude + '.json';
  
  console.log('Lat = ' + pos.coords.latitude);
  console.log('Long = ' + pos.coords.longitude);
  
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      
      // Temperature in Kelvin requires adjustment
      // Temp for openweathermap
      var temperature = Math.round((json.main.temp - 273.15) * 1.8 + 32);
      // Temp for wunderground
      //var temperature = json.current_observation.temp_f;
      console.log('Temperature is ' + temperature);
      
      // Conditions
      // Conditions for openweathermap
      var conditions = json.weather[1].main;
      // Conditions for wunderground
      //var conditions = json.current_observation.weather;
      console.log('Condition is ' + conditions);
      
      var currentdate = new Date(); 
      var n = currentdate.getHours() + ":" + ((currentdate.getMinutes()<10) ? "0" + currentdate.getMinutes() : currentdate.getMinutes() );
      console.log("Time: " + n);
      
      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_TEMPERATURE': temperature,
        'KEY_CONDITIONS': conditions,
        'KEY_UPDATE': n
      };
    
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Weather info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending weather info to Pebble!');
        }
      );
  });
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function(e) {
    console.log('PebbleKit JS ready!');
    getWeather();
});

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
});


  
