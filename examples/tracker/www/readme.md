# Example Tracker Web Application

This is an example of how you can write your own web application and use Myriota Cloud API to get the data of your devices. It shows location and timestamp of your devices which has been programmed with tracker user application built from tracker example C code. This web application is also hosted by [Myriota](https://apps.myriota.com/tracker).

## Google Map API

The application will run without the Google API key but expect to see an error message with an unattractive map.
You may acquire a [Google API Key](https://developers.google.com/maps/documentation/embed/get-api-key), which can be free. Then append `?key=<YOUR GOOGLE API KEY>` to the link `https://maps.googleapis.com/maps/api/js` in `index.html` file.

## Run On Local Computer

Simply open `index.html` file with your browser.
