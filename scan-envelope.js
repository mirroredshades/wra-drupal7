// JavaScript Document
var WebTWAIN;// Set a global variable for the DWT object
var seed;//Set a global variable for time interval
window.onload = Pageonload; // Make sure the code executes after the page is loaded

function Pageonload() {
         /*
         Initialize the webpage, do not call Dynamic Web TWAIN directly
         */
//		 alert('test1');
         WebTWAIN = document.embeds[0]; //Make sure it's the right id for the object
         seed = setInterval(ControlDetect, 500);//Wait 500ms after opening the webpage, call "ControlDetect" function
}

function ControlDetect() {
         if (WebTWAIN.ErrorCode == 0) {//Here we use "WebTWAIN.ErrorCode" to determine if the object is fully initialized
         clearInterval(seed); //If DWT is fully initialized, clear the interval  
		 AcquireImage();
         }
         timeout = setTimeout(function () { }, 10);//If the initiation of DWT is not ready, retry after 10ms
}

function AcquireImage(){
	// scan image
	var WebTWAIN = document.embeds[0];
	WebTWAIN.SelectSourceByIndex(0);
	WebTWAIN.CloseSource();
	WebTWAIN.OpenSource();
	WebTWAIN.IfShowUI = 0;
	WebTWAIN.PixelType = 0;
	WebTWAIN.Resolution = 150;
	WebTWAIN.IfDisableSourceAfterAcquire = true;
	document.getElementById("edit-upload").disabled = true;
	document.getElementById("edit-form-factor").disabled = false;
	document.getElementById("edit-form-factor").value = '';
	document.getElementById("edit-save").disabled = true;
	WebTWAIN.RemoveAllImages();
	WebTWAIN.AcquireImage();
	WebTWAIN.RotateLeft(0);
	
}

function SelectSize(){
//	alert("test");
	var WebTWAIN = document.embeds[0];
	switch(document.getElementById("edit-form-factor").value) {
		case 'envelope#10' :
			WebTWAIN.Crop(0, 0 ,0,(9.5*150),(4.125*150));
			break;

		case 'envelope6x9' :
//			WebTWAIN.Crop(0, (2.75 * 150),0,(8.5*150),(9*150));
			WebTWAIN.Crop(0, 0, 0,(9*150),(6*150));
			break;
		
		case 'envelope9x12' :
			break;
	}
	document.getElementById("edit-form-factor").disabled = true;
	document.getElementById("edit-upload").disabled = false;
	document.getElementById("edit-save").disabled = true;
}

function Upload(){
	var WebTWAIN = document.embeds[0];
	document.getElementById("edit-form-factor").disabled = true;
	document.getElementById("edit-upload").disabled = true;
	document.getElementById("edit-save").disabled = false;

//	alert("test");
	
	var ret = WebTWAIN.HTTPUploadThroughPut('my.wyomingregisteredagent.com', 0, '/wysos/suite/upload-envelope/'+document.getElementById("edit-unique-id").value+'.PNG');
//	var ret = WebTWAIN.HTTPUploadThroughPost('my.wyomingregisteredagent.com', 0, '/wysos/suite/upload-envelope/2/', 'test.PNG');
//	alert('ret: '+ret.toString()+' Err: '+WebTWAIN.ErrorCode.toString()+': '+WebTWAIN.ErrorString);

//    seed = setInterval(UploadComplete, 500);//Wait 500ms after opening the webpage, call "ControlDetect" function

	return ret;
}

function UploadComplete() {
         if (WebTWAIN.ErrorCode == 0) {//Here we use "WebTWAIN.ErrorCode" to determine if the object is fully initialized
         clearInterval(seed); //If DWT is fully initialized, clear the interval  
		 AcquireImage();
         }
         timeout = setTimeout(function () { }, 10);//If the initiation of DWT is not ready, retry after 10ms
}
