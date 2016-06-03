<?php
	$stdout = fopen('php://stdout', 'w');
	fwrite($stdout, "Started ".date("l dS \of F Y h:i:s A")."\r\n");
	
	// Site specific variables
	$username = "ron";
	$drupal_base_url = parse_url('http://my.wyomingregisteredagent.com');
	
	$_SERVER['HTTP_HOST'] = $drupal_base_url['host'];
	$_SERVER['PHP_SELF'] = $drupal_base_url['path'].'/index.php';
	$_SERVER['REQUEST_URI'] = $_SERVER['SCRIPT_NAME'] = $_SERVER['PHP_SELF'];
	$_SERVER['REMOTE_ADDR'] = NULL;
	$_SERVER['REQUEST_METHOD'] = NULL;
	
	// change to the Drupal directory
	chdir('/home/wyregne/public_html');
	define('DRUPAL_ROOT', getcwd());
	
	require_once 'includes/bootstrap.inc';
	drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
	
	global $user;
	$user = user_load(array('name' => $username));
	
	include_once('sites/all/modules/wysos/wysos.util.inc');

	$doc = new DOMDocument();
	$load = @$doc->loadHTMLFile("https://wyobiz.wy.gov/Business/Database.aspx");
	
	$inputs = $doc->getElementsByTagName('input');
    foreach($inputs as $input) {
//		echo 'inner html: '.get_inner_html($input)."\r\n";
//		echo 'saveXML: '.$doc->saveXML($input)."\r\n";
		$data[$input->getAttribute('name')] = $input->getAttribute('value');
//		echo $input->getAttribute('name') .'=>'.$input->getAttribute('value');
    }
	$data['ctl00$contentMain$DownloadButton'] = 'Download';
	unset($doc);
	echo "Posting...\r\n";
	flush();
	
	$url = "https://wyobiz.wy.gov/Business/Database.aspx";
	$ch = curl_init();
	curl_setopt ($ch, CURLOPT_URL, $url);
	curl_setopt ($ch, CURLOPT_USERAGENT, "Mozilla/5.001 (windows; U; NT4.0; en-us) Gecko/25250101");
	curl_setopt ($ch, CURLOPT_HEADER, 0);
	curl_setopt ($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_POST, true);
	curl_setopt ($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
	
	curl_setopt($ch, CURLOPT_POSTFIELDS, $data);		
	if($raw = curl_exec ($ch)) {
		echo "Putting...\r\n";
		flush();
		echo 'Contents: ' . substr($raw, 1, 200);
		flush();
//		if(!file_put_contents("/home/wyregne/public_html/docs/Database123.zip", $raw)) {
//			echo "Error\r\n";
//			return;
//		}
//		$doc = new DOMDocument();
//		$load = @$doc->loadHTML($raw);
		
//		echo $raw;
		
//		unset($doc);
	}




	echo "Done\r\n";
	flush();

?>