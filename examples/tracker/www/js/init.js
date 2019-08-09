/***********************
Global constants.
************************/
API_V1_DOMAIN = "https://api.myriota.com/v1/"
AUTH_DOMAIN = "https://cognito-idp.us-east-1.amazonaws.com/"

/***********************
DeviceDataStore is the main app data model and stores all app device data.

It uses the Myriota API to:

	1) get a list of devices for the authorised account,
	2) for each device, loads the last message that was logged by the devices and,
	3) unpacks the message into the equivalent location data.

For every successful 'get' of a device message, calls DeviceList and
DeviceMap functions to add the device to the list and render the marker on the map.
************************/
var DeviceDataStore = (function() {

	/***********************
	 Private state.
    ************************/
    var state = null;

    /***********************
	 Private functions.
    ************************/

    /*
	 Converts the string hex value into an integer array. Note, any exceptions are
	 caught by the callers try, catch block.
    */
    function toIntArray(str) {
		var result = [];
		for(var i = 0, length = str.length; i < length; i+=2) {
		    var code = str.substring(i, i+2);
		    result.push(parseInt(code, 16));
		}
		return result;
	}

    /*
	 Unpacks the integer array into it's value components. Note, any exceptions are
	 caught by the callers try, catch block.
    */
	function unpack(value)  {
		var res = jspack.Unpack("<HiiI", toIntArray(value));
		return {"messageNum":res[0], "position":{lat:res[1]/1e7, lng:res[2]/1e7}, "timestamp":res[3]}
	}

 	/*
 	 Calls the Myriota data/ endpoint to get the last message for the given device.
 	*/
 	function fetchLastMessageForDevice(deviceId) {
 		$.ajax({
		    'url': API_V1_DOMAIN + "data/" + deviceId + "/Message?limit=1",
		    'type': 'get',
		    'headers': headers
		}).done(function (data) {
	    	if (!data || !data.Items || data.Items.length === 0)
	    		state.deviceData[deviceId] = {"messages":null,"error":true,"errorInfo":"No location data available."};
	    	else {
	    		// For each message (there's only one in this case), try and unpack the hex 'Value' into the location fields.
	    		try {
	    			data.Items.forEach((d) => d.Value = unpack(d.Value));
		    		state.deviceData[deviceId] = {"messages":data.Items,"error":false};
		    	}
		    	catch (err) {
		    		state.deviceData[deviceId] = {"messages":null,"error":true};
		    	}
		    }
		    // Call the data loaded callbacks.
			state.deviceDataLoadedCb.forEach((cb) =>
				cb(deviceId, state.deviceData[deviceId], ++state.numDevicesLoaded, Object.keys(state.deviceData).length)
			);
		})
		.fail(function (error) {
			state.deviceData[deviceId] = {"messages":null,"error":true,"errorInfo":error};
		    // Call the data loaded callbacks (even on an error).
			state.deviceDataLoadedCb.forEach((cb) =>
				cb(deviceId, state.deviceData[deviceId], ++state.numDevicesLoaded, Object.keys(state.deviceData).length)
			);
		})
 	}

    /***********************
	 Public functions.
    ************************/

    /*
    init resets the module to it's initial state. Must be called on every successful auth.
    */
    var init = function() {
	    state = {
    		deviceData:{}, // location data by device id
    		deviceDataLoadedCb: [], // list of functions to call when device data is loaded
    		deviceDataLoadFailedCb:null, // function to call when device data fails to load
    		numDevicesLoaded: 0 // keeps track of how many devices have been loaded
    	}
	}

	/*
	Kicks off the process of loading device data using the Myriota API. Gets a list of
	devices for the current account, then for each device, it's last stored message.
	*/
	var loadData = function() {
		var IdToken = localStorage.getItem("IdToken");

		headers = {'Content-type': 'application/json', 'Authorization': IdToken}

		$.ajax({
		    'url': API_V1_DOMAIN + 'modules',
		    'type': 'get',
		    'headers': headers,
		}).done(function(data) {
			if (!data.Items || data.Items.length==0) {
				state.deviceDataLoadFailedCb();
			}
			else {
				data.Items.forEach((t) => {
					state.deviceData[t["Id"]] = {"messages":null, "error":true};
					fetchLastMessageForDevice(t["Id"]);
				});
			}
		}).fail(function(data) {
			state.deviceDataLoadFailedCb();
		});
	}

	var getDeviceData = function(id) {
		return state.deviceData[id] || null;
	}

	/*
	 Registers the function to be called every time location data for a device
	 has been fetched and unpacked. Used by DeviceMap and DeviceList.
	*/
	var onSingleDeviceDataLoadedCb = function(cb) {
		state.deviceDataLoadedCb.push(cb);
	}

 	/*
	 Registers the function to be called on a failure to get the device data.
	*/
	var onDeviceDataLoadFailedCb = function(cb) {
		state.deviceDataLoadFailedCb = cb;
	}

    /***********************
	 Public API.
    ************************/
    return {
        init: init,
        loadData: loadData,
        getDeviceData: getDeviceData,
        onSingleDeviceDataLoadedCb: onSingleDeviceDataLoadedCb,
		onDeviceDataLoadFailedCb: onDeviceDataLoadFailedCb
    };

})();


/***********************
DeviceList is the controller for the HTML list of devices
(see <div id="device-list">...</div>). It renders the list items,
(see <ul class="collection">...</ul>) and listens for user inputs such
as list item clicks and hide/show switch toggles.
************************/
var DeviceList = (function() {

	/***********************
	Private state.
    ************************/
    var state = null;


    /***********************
	 Private functions.
    ************************/

    /*
	Returns the item location detail HTML snippet e.g.
	<p style="font-size:10px">Location on 19/06/2019
		<br>3:17:43 PM:
		-33.26191, 138.077785
	</p>
	*/
	function listItemDetailHtml(deviceData) {
		// Message timestamp is in seconds
		var date = new Date(deviceData["messages"][0].Value.timestamp * 1000);
		var lat = deviceData["messages"][0].Value.position.lat;
		var lng = deviceData["messages"][0].Value.position.lng;
		var div = [
			'<p style="font-size:10px">Location on '+date.toLocaleDateString(),
			'<br>'+date.toLocaleTimeString('en-US')+': ',
			lat + ', ' + lng,
			'</p>'
		].join("\n");
		return div
	}

	/*
	Returns the item detail HTML snippet e.g.
	<div>
		<span class="title">0090e66171</span>
		<p style="font-size:10px">Location on 19/06/2019
		<br>3:17:43 PM:
		-33.26191, 138.077785
		</p>
	</div>
	*/
	function listItemHtml(id, deviceData) {
		var div;
		if (deviceData.error) {
			div = [
			'<div>',
			'<span class="title">' + id + '</span>',
			'<p style="font-size:10px">No location data available.',
			'</p></div>'
			].join("\n");
		} else {
			div = [
			'<div>',
			'<span class="title">' + id + '</span>',
			listItemDetailHtml(deviceData),
			'</div>'
			].join("\n");
		}
		return div;
	}

	/*
		Adds the list item HTML to <ul class="collection">...</ul>
	*/
	var addListItem = function(id, deviceData, numItemsLoaded, totalItems)
	{
		var div = listItemHtml(id, deviceData);
		// Render hide/show toggle button for device with location data only
		var toggleButton = deviceData.error === false ? [
			'<div class="secondary-content">',
			'    <div class="switch">',
			'      <label>',
			'        Show',
			'        <input type="checkbox">',
			'        <span class="lever" style="margin:5px"></span>',
			'        Hide',
			'      </label>',
			'    </div>',
			'</div>'
		].join("\n") : "";

		var html = [
		    '<li id="' + id + '" class="device-list-item collection-item avatar" style="cursor: pointer;">',
		    '<i class="material-icons circle">location_on</i>',
			div,
			toggleButton,
		    '</li>',
		].join("\n");

		var htmlListElement = $(html);
		var switchElement = $(".secondary-content", htmlListElement);

		//.click was causing the event to fire twice.
		switchElement.on("change", {"id": id}, function(event) {
			onDeviceVisibilityToggled(event.data.id);
		});

		// This prevents the list item from being selected
		// when the on/off switch is clicked.
		switchElement.click(function(event){
			event.stopPropagation();
		})

		state.device[id] = {"htmlListElement":htmlListElement, "isSelected":false, "isVisible":false};

		state.device[id].htmlListElement.click({"id": id}, function(event) {
			onDeviceSelected(event.data.id);
		});

		$(".collection").append(state.device[id].htmlListElement);
		$(".list").show();

		// Calculates the progress of the device data load operation.
		// totalItems is the total number of unique devices who's location
		// messages are being fetched.
		var progressPercent = (numItemsLoaded/totalItems)*100;
		if (progressPercent>=100)
			$(".list-progress").hide();
	}

    /***********************
	 Public functions
    ************************/

    /*
    init resets the module to it's initial state. Must be called on every successful auth.
    */
    var init = function(deviceActiveCb, deviceVisibilityToggledCb) {

        // Clear the list HTML
        $(".collection").empty();
        // Hide the list until the first item is loaded.
        $(".list").hide();
        // Show the progress loader.
		$(".list-progress").show();
		// Reset the error message.
		$("#device-load-error").hide();

	    state = {
	    	activeDevice: null, // list item that a user has selected
	    	device: {}, // state data by device
	    	onDeviceActiveCb: null, // function to call when an item is selected
	    	onDeviceVisibilityToggledCb: null, // function to call when an items switch is toggled
	    	onLoadFailedCb: null // function which is called when device data fails to load
	    };

		$("#device-list-filter").on("keyup", function() {
		   	$('.collection > li:not(:contains(' + $(this).val() + '))').hide();
			$('.collection > li:contains(' + $(this).val() + ')').show();
		});
	}

	/*
	Called when the visibility switch is toggled for a list item.
	*/
	var onDeviceVisibilityToggled = function(id) {
		state.device[id].isVisible = !state.device[id].isVisible;

		// Call the listener.
		state.onDeviceVisibilityToggledCb(id);

	}

	/*
	Called when the list item is clicked.
	*/
	var onDeviceSelected = function(id) {
		// Unselect the currently selected device.
		if (state.activeDevice) {
			state.activeDevice.isSelected = false;
			state.activeDevice.htmlListElement.toggleClass('active');
		}
		// Make the new device the currently selected device.
		state.device[id].isSelected = true;
		state.device[id].htmlListElement.toggleClass('active');
		state.activeDevice = state.device[id];

		// Call all the listener.
		state.onDeviceActiveCb(id);
	}

 	/*
 	Registers the function to be called when a list item is clicked.
 	*/
	var onItemActiveCb = function(cb) {
		state.onDeviceActiveCb = cb;
	}

 	/*
 	Registers the function to be called when a list item switch is toggled.
 	*/
	var onItemSwitchToggledCb = function(cb) {
        state.onDeviceVisibilityToggledCb = cb;
	}

	/*
 	Displays the error message if device data load fails.
 	*/
	var onLoadFailedCb = function(cb) {
    	$("#device-load-error").show();
    	$(".list-progress").hide();
	}

	/**
	* To be attached to onclick event
	*/
	function sortFunc(event) {
		// stop click event propagating through
		event.stopPropagation();
		var deviceList = $("#deviceList");
		var elements = deviceList.children("li");
		// sort by alphabetical order
		if (event.target.id === "a2z") {
			elements.sort(function(a, b) {
				if (a.id < b.id) {
					return -1;
				} else {
					return 1;
				}
			});
		}
		// sort by whether a device has location data
		else if (event.target.id === "location") {
			elements.sort(function(a, b) {
				// devices with location data would have toggle button
				var aEl = $(".switch", a);
				var bEl = $(".switch", b);
				if (aEl.length > 0 && bEl.length === 0) {
					return -1;
				} else {
					return 1;
				}
			});
		}
		// update sort label
		$("span#sortLabel")[0].innerText = event.target.text;
		// update list
		deviceList.append(elements);
	}

    /***********************
	 Public API
    ************************/
    return {
        init: init,
        addListItem: addListItem,
		onItemActiveCb: onItemActiveCb,
		onItemSwitchToggledCb: onItemSwitchToggledCb,
		onLoadFailedCb: onLoadFailedCb,
		sort: sortFunc
    };

})();

/***********************
DeviceMap is the controller for the Google Map. It renders the map marker,
for each device location. It displays an info window when the marker is
clicked.
***********************/
var DeviceMap = (function() {

	/***********************
	Private state.
    ************************/
    var state = null;

    var map = new google.maps.Map(document.getElementById('device-map'), {
        	center: {lat: -34.92, lng: 138.60},
        	zoom: 2
    });
    var bounds = new google.maps.LatLngBounds();
 	var infowindow = new google.maps.InfoWindow();

 	/***********************
	Private functions.
    ************************/
	function onMarkerClick() {
	 	infowindow.setContent(this.info);
	 	infowindow.marker = this;
	 	infowindow.open(map, this);
	}

	function infoWindoHtml(id, deviceData) {
		// Message timestamp is in seconds
		var date = new Date(deviceData["messages"][0].Value.timestamp * 1000);
		var lat = deviceData["messages"][0].Value.position.lat;
		var lng = deviceData["messages"][0].Value.position.lng;
		div = [
		'<div',
		'<span class="title">' + id + '</span>',
		'<p style="font-size:10px">Location on '+date.toLocaleDateString(),
		' '+date.toLocaleTimeString('en-US'),
		'<br>' + lat + ', ' + lng,
		'</p></div>'
		].join("\n");

		return div;
	}

 	/***********************
	Public functions.
    ************************/
    var init = function(deviceData) {

    	// Remove old map markers if any.
    	if (state && state.device) {
    		for (var idx in state.device)
    			if (state.device[idx].marker) {
    				state.device[idx].marker.setMap(null);
    				state.device[idx].marker = null;
    			}
    	}

    	// Init state.
		state = {
    		activeDevice: null,
    		device: {}
    	}

	}

	/*
	Adds the google map marker at the last reported lat/long
	coordinates for a single device. Sets up the on click events
	and extends the map bounds to initially capture all markers in the
	map viewport.
	*/
	var addMapMarker = function(id, deviceData) {
		if (deviceData.error)
			return;

		var infoWindowHtml = infoWindoHtml(id, deviceData)

		var markerIcon = {
			labelOrigin: new google.maps.Point(10, -10),
			url: "https://maps.google.com/mapfiles/ms/icons/blue-dot.png"
		}

		var m = {
			icon: markerIcon,
			label:id,
			info:infoWindowHtml,
			position: deviceData.messages[0].Value.position,
			map: map
		};
		var marker = new google.maps.Marker(m);

		state.device[id] = {marker:marker};
		google.maps.event.addListener(marker, 'click', onMarkerClick);

		// Results in zoom level that captures all markers.
		bounds.extend(marker.getPosition());
		map.fitBounds(bounds);
	}

	var toggleDeviceVisibility = function(id) {
		if (state.device[id]) {
			state.device[id].marker.setVisible(!state.device[id].marker.visible);
			// if currently opened info window is for this marker and it's been hidden
			// close the info window too
			if (state.device[id].marker === infowindow.marker) {
				if (!state.device[id].marker.visible)
					infowindow.close();
			}
		}
	}

	var centreOnDevice = function(id) {
		if (state.device[id])
			map.setCenter(state.device[id].marker.getPosition());
	}

	var getState = function() {
		return state;
	}

 	/***********************
	Public API.
    ************************/
    return {
        init: init,
        centreOnDevice: centreOnDevice,
        toggleDeviceVisibility: toggleDeviceVisibility,
        addMapMarker: addMapMarker,
        getState: getState
    };

})();

/***********************
Auth is responsible for app user
authentication.
***********************/
var Auth = (function() {

	/***********************
	Private state.
    ************************/
    var state = {
		onAuthSuccessCb:null, // Function to call if auth succeeds.
		onAuthFailCb:null, // Function to call if auth fails.
		onAuthSignOutCb:null // Function to call on auth sign out request.
	};

	/***********************
	Private functions.
    ************************/
    function authWithEmailAndPassword(email, password) {
	    var data = {
	        "AuthFlow": "USER_PASSWORD_AUTH",
	        "ClientId": "4jskgo1eq6ngcimlseerg50uvd",
	        "AuthParameters" : {'USERNAME': email, 'PASSWORD': password}
	    }
	    auth(data).then(null,
	    	function(){
	    		$( "#sign-in-error").show();
	    	}
	    );
    }

    function auth(data) {
    	var headers = {
	        "Content-Type": "application/x-amz-json-1.1",
	        "X-Amz-Target": "AWSCognitoIdentityProviderService.InitiateAuth"
	    }
		return $.ajax({
		    'url': AUTH_DOMAIN,
		    'type': 'post',
		    'data': JSON.stringify(data),
		    'headers': headers,
		    'dataType': 'json'
		}).done(function(data){
			if (data["AuthenticationResult"] && data["AuthenticationResult"]["IdToken"]) {
		        localStorage.setItem("IdToken", data['AuthenticationResult']['IdToken']);
		        if (data['AuthenticationResult']['RefreshToken'])
		        	localStorage.setItem("RefreshToken", data['AuthenticationResult']['RefreshToken']);
		        state.onAuthSuccessCb();
		    }
		    else {
		    	state.onAuthFailCb();
		    }
		}).fail(function(){
			state.onAuthFailCb();
		});
    }

	/***********************
	Public functions.
    ************************/
    var init = function(onAuthSuccessCb_, onAuthFailCb_, onAuthSignOutCb_) {
    	state.onAuthSuccessCb = onAuthSuccessCb_;
    	state.onAuthFailCb = onAuthFailCb_;
    	state.onAuthSignOutCb = onAuthSignOutCb_;

		$("#sign-in").click(function() {

			var email = $("#email").val();
			var password = $("#password").val();

		    authWithEmailAndPassword(email,password);
		});

		$("#sign-out").click(function() {
			localStorage.removeItem("RefreshToken");
			localStorage.removeItem("IdToken");
			state.onAuthSignOutCb();
		});

		// capture user enter key press
		$('#email,#password').keypress(function (e) {
		  if (e.which == 13) {
			var email = $("#email").val();
			var password = $("#password").val();

		    authWithEmailAndPassword(email,password);
		    return false;
		  }
		});
    }

    /*
    Attempt to authenticate with the stored token.
    */
    var authWithSavedToken = function() {
	    var data = {
	        "AuthFlow": "REFRESH_TOKEN_AUTH",
	        "ClientId": "4jskgo1eq6ngcimlseerg50uvd",
	        "AuthParameters" : {'REFRESH_TOKEN': localStorage.getItem("RefreshToken")}
	    }

	    auth(data);

    }

 	/***********************
	Public API.
    ************************/
    return {
        init: init,
        authWithSavedToken: authWithSavedToken
    };

})();

/***********************
Top level functions. The app is driven by the Auth state.
************************/
function onAuthSuccess()
{
	$( ".state-signed-out" ).hide();
	$( ".state-signed-in" ).show();

	// Clear the signup forms.
	$("#email").val("");
	$("#password").val("");
	$( "#sign-in-error").hide();

	// On mobile browsers, the sliding keyboard pushes the content up
	// and pushes the nav bar out of view on some devices. Make sure we start off
	// scrolled all the way to the top.
	window.scrollTo(0, 0);

	DeviceList.init();
	DeviceList.onItemActiveCb(DeviceMap.centreOnDevice);
	DeviceList.onItemSwitchToggledCb(DeviceMap.toggleDeviceVisibility);

	DeviceMap.init();

	DeviceDataStore.init();
	DeviceDataStore.onSingleDeviceDataLoadedCb(DeviceList.addListItem);
	DeviceDataStore.onSingleDeviceDataLoadedCb(DeviceMap.addMapMarker);
	DeviceDataStore.onDeviceDataLoadFailedCb(DeviceList.onLoadFailedCb);
	DeviceDataStore.loadData();
}

function onAuthFail() {
	$( ".state-signed-out" ).show();
	$( ".state-signed-in" ).hide();
}

function onAuthSignOut() {
	$( ".state-signed-out" ).show();
	$( ".state-signed-in" ).hide();
}

/******************************************
Top level entry point. We always attempt to
auth with the saved token once the JS is
initialised.
******************************************/
(function($){

  $(function() {
	Auth.init(onAuthSuccess, onAuthFail, onAuthSignOut);
	Auth.authWithSavedToken();
	// This check is for Jest unit testing so that we
	// don't have to load the materialize js library just
	// for this one call.
	if ($('.sidenav').sidenav)
		$('.sidenav').sidenav();

	if ($('.dropdown-trigger').dropdown) {
		$('.dropdown-trigger').dropdown();
		// attach sort function to onClick event of each item in drop down list
		let dropdownEl = $("a", $(".dropdown-content"));
		for (let i = 0; i < dropdownEl.length; i++) {
			dropdownEl[i].onclick = DeviceList.sort;
		}
	}

  }); // End of document ready.
})(jQuery); // End of jQuery name space.

/**********************************
Module exports for Jest unit testing.
***********************************/
if (typeof exports !== 'undefined') {
    module.exports = { Auth, DeviceMap, DeviceList, DeviceDataStore, onAuthSuccess };
}
