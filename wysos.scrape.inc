<?php 
function wysos_scrape_test_form($form, &$form_state)
{
	$form['cid'] = array(
		'#type' => 'textfield',
		'#title' => 'CID',
		'#size' => '20',
		'#maxlength' => '20',
		);
		$form['get-efnum'] = array (
			'#type' => 'submit',
			'#value' => t('Get eFnum')
		);

	return $form;
}

function wysos_scrape_test_form_submit($form, &$form_state)
{
	switch($form_state['clicked_button']['#id'])
	{
		case 'edit-get-efnum' :
			if($eFnum = wysos_scrape_eFNum($form_state['values']['cid'])) {
				echo 'eFnum='.$eFnum;
			} else {
				echo 'eFnum=FALSE';
			};
			break;
	}
}
function wysos_scrape_eFNum($cid)
{
//	drupal_set_message('wysos_get_efnum_from_cid('.$cid.')');

	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL,"https://wyobiz.wy.gov/Business/FilingSearch.aspx");
	curl_setopt ($ch, CURLOPT_USERAGENT, "Mozilla/5.001 (windows; U; NT4.0; en-us) Gecko/25250101");
//	curl_setopt($ch, CURLOPT_PROXY, $proxy);
	curl_setopt ($ch, CURLOPT_HEADER, 1);
	curl_setopt ($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt ($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
	$raw = curl_exec($ch);
	$pos = strpos($raw,'<!DOCTYPE html');
	$headers = trim(substr($raw,0,$pos-2));
	$body = trim(substr($raw,$pos));
	curl_close ($ch);
	unset($ch);	
	// Set Cookies
	preg_match_all('|Set-Cookie: (.*);|U', $headers, $results);    
	$cookies = implode(';', $results[1]);
	echo 'cookies: '.print_r($cookies, true).'<br />';
	// Extract html

	$doc = new DOMDocument();
	libxml_use_internal_errors(true);
	$load = $doc->loadHTML($body);
	libxml_use_internal_errors(false);
	$inputs = $doc->getElementsByTagName('input');
    foreach($inputs as $input) {
		$data[$input->getAttribute('name')] = $input->getAttribute('value');
    } 
	$data['ctl00$MainContent$myScriptManager'] = 'ctl00$MainContent$UpdatePanel1|ctl00$MainContent$cmdSearch';
	$data['__LASTFOCUS'] = urlencode('');
	$data['__EVENTTARGET'] = '';
	$data['__EVENTARGUMENT'] = urlencode('');
	$data['ctl00$MainContent$txtFilingName'] = '';
	$data['ctl00$MainContent$searchOpt'] = 'chkSearchStartWith';
	$data['ctl00$MainContent$txtFilingID'] = $cid;
	$data['__ASYNCPOST'] = 'true';
	$data['ctl00$MainContent$cmdSearch'] = ' Search'; 
	unset($doc);
	
	$url = "https://wyobiz.wy.gov/Business/FilingSearch.aspx";
	$ch = curl_init();
	curl_setopt ($ch, CURLOPT_URL, $url);
	curl_setopt($ch, CURLOPT_COOKIE,  $cookies);
	curl_setopt ($ch, CURLOPT_USERAGENT, "Mozilla/5.001 (windows; U; NT4.0; en-us) Gecko/25250101");
	curl_setopt ($ch, CURLOPT_HEADER, 0);
	curl_setopt($ch,CURLOPT_HTTPHEADER,array('X-Requested-With: XMLHttpRequest', 'X-MicrosoftAjax: Delta=true', 'Content-Type: application/x-www-form-urlencoded; charset=utf-8', 'Origin: https://wyobiz.wy.gov'));
//	curl_setopt($ch, CURLOPT_PROXY, $proxy);
	curl_setopt ($ch, CURLOPT_REFERER, 'https://wyobiz.wy.gov/Business/FilingSearch.aspx');
	curl_setopt ($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_POST, true);
	curl_setopt ($ch, CURLOPT_FOLLOWLOCATION, true); // follow redirects 
	curl_setopt ($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_setopt( $ch, CURLOPT_HTTPHEADER, array( 'Expect:' ) );
	
	curl_setopt($ch, CURLOPT_POSTFIELDS, $data);		
	if($raw = curl_exec ($ch)) {
		$needle = 'FilingDetails.aspx?eFNum=';
		$start = strpos($raw,$needle) + strlen($needle);
		$end = strpos($raw,"'>",$start);
		$eFnum = substr($raw,$start,($end-$start));
		if(empty($eFnum)) {
			return FALSE;
		} else {
			//db_query("UPDATE ra_companies SET eFNum='".$eFnum."' WHERE filing_num='".$cid."'");
			db_update('wra_companies')->fields(array('eFNum'=>$eFnum))->condition('filing_num',$cid,'=')->execute();
		}
	}  else {
		drupal_set_message('Curl Error ('. curl_errno($ch) .'): ' .curl_error ( $ch ));
		return FALSE;
	}
	return $eFnum;
}
