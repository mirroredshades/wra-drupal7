<?php 
function wysos_autocomplete_suite_no_lookup($string = NULL){
	 $matches = array();
		$result = db_query_range("SELECT c.company_name, c.mail_action, s.suite_no
		FROM wra_companies c
		INNER JOIN wra_suites s ON s.company_id = c.company_id
		WHERE LOWER(s.suite_no) LIKE LOWER(:suite_no)
		AND (c.scraped_ra_id=249 OR c.suite_id IS NOT NULL)
		ORDER BY s.suite_no", 0, 30,array(':suite_no'=>'%'.db_like($string)))->fetchAll();
	foreach ($result as $filing) {
		$matches[$filing->suite_no] = check_plain($filing->suite_no .' - '.$filing->company_name.' ['.$filing->mail_action.']');
	} 
	
	 print drupal_json_encode($matches);
	exit();

}
function wysos_autocomplete_person_lookup($string = NULL) {
	$matches = array();
		$result = db_query_range("SELECT CONCAT(a.last_name,', ',a.first_name) AS full_name, a.uid
		FROM uc_addresses a
		INNER JOIN wra_companies c ON a.uid=c.uid
		WHERE LOWER(CONCAT(a.last_name,', ',a.first_name)) LIKE LOWER(:name)
		AND (c.scraped_ra_id=249 OR c.suite_id IS NOT NULL OR c.mail_action IS NOT NULL)
		ORDER BY full_name, company_name", 0, 30,array(':name'=> '%' .db_like($string)))->fetchAll(); 
		foreach ($result as $person) {
			$matches[$person->full_name] = check_plain($person->full_name);
		} 
	print drupal_json_encode($matches);
	exit();
}
function wysos_email_autocomplete($string = NULL){
	$matches = array();
		$result = db_query_range("SELECT mail FROM users WHERE LOWER(mail) LIKE LOWER(:mail) ORDER BY mail",0,10,array(':mail' =>db_like($string).'%'))->fetchAll();
		foreach($result as $user){
			$matches[$user->mail] = check_plain($user->mail);
		}
	print drupal_json_encode($matches);
	exit();
}
function wysos_email_user_autocomplete($string = NULL){
	$matches = array();
	$result = db_query_range("SELECT mail AS value FROM users WHERE LOWER(mail) LIKE LOWER(:mail) UNION SELECT name as value FROM users WHERE LOWER(name) LIKE LOWER(:name) ORDER BY value", 0,16, array(':mail'=>'%'. db_like($string),':name'=>'%'. db_like($string)))->fetchAll();
	foreach($result as $user){
		$matches[$user->value] = check_plain($user->value);
	}
	print drupal_json_encode($matches);
	exit();
}
function wysos_autocomplete_client_company($string = NULL) {
  $matches = array();

  print drupal_json_encode($matches);
  exit();
}
function wysos_autocomplete_all_companies($string = NULL) {
  $matches = array();
  $result = db_query_range("SELECT company_name FROM wra_search_all_companies WHERE LOWER(company_name) LIKE LOWER(:company_name) ORDER BY company_name", 0, 16, array(':company_name'=>'%'.db_like($string)))->fetchAll();
  
  foreach ($result as $company) {
    $matches[$company->company_name] = check_plain($company->company_name);
  }
  print drupal_json_encode($matches);
  exit();
}
function wysos_entity_search_autocomplete($string = NULL) {
	$matches = array();
	$result = db_query_range("SELECT filing_name FROM wysosuserfiling WHERE LOWER(filing_name) LIKE LOWER(:filing_name) 
	UNION
	SELECT company_name AS filing_name FROM wra_mail_suites WHERE LOWER(company_name) LIKE LOWER(:company_name) AND company_name NOT IN(SELECT filing_name FROM wysosuserfiling WHERE LOWER(filing_name) LIKE LOWER(:f_name))",
	0,16,array(':filing_name' => '%'.db_like($string),'company_name'=>'%'.db_like($string), ':f_name' => '%'.db_like($string)))->fetchAll();
	foreach($result as $user){
		$matches[$user->filing_name] = check_plain($user->filing_name);
	}
	print drupal_json_encode($matches);
	exit();
}
function wysos_filing_autocomplete($string = NULL) {
  $matches = array();
  $result = db_query_range("SELECT FILING_NAME FROM wysosfiling WHERE LOWER(FILING_NAME) LIKE LOWER(:filing_name) 
  AND (STATUS IN('Active','Delinquent') 
  OR (STATUS IN('Inactive - Administratively Dissolved (Tax)','Inactive - Administratively Dissolved (No Agent)') AND DATEDIFF(INACTIVE_DATE,CURRENT_DATE()) < 761)) 
  AND FILING_TYPE IN('Profit Corporation','NonProfit Corporation','Limited Liability Company','Limited Partnership',
  'Statutory Trust','Registered Limited Liability Partnership') ORDER BY FILING_NAME", 0, 16,array(':filing_name' => db_like($string).'%'))->fetchAll();
  
  foreach ($result as $user) {
    $matches[$user->FILING_NAME] = check_plain($user->FILING_NAME);
  }
  print drupal_json_encode($matches);
  exit();
}
function wysos_reinstate_autocomplete($string = NULL) {
  $matches = array();
  $result = db_query_range("SELECT FILING_NAME, FILING_NUM FROM wysosfiling WHERE LOWER(FILING_NAME) LIKE LOWER(:FILING_NAME) 
  AND STATUS IN('Inactive - Administratively Dissolved (Tax)','Inactive - Administratively Dissolved (No Agent)') 
  AND DATEDIFF(INACTIVE_DATE,CURRENT_DATE()) < 730 AND FILING_TYPE IN('Profit Corporation','NonProfit Corporation','Limited Liability Company','Limited Partnership',
  'Statutory Trust','Registered Limited Liability Partnership') ORDER BY FILING_NAME", 0, 16, array(':FILING_NAME'=>db_like($string).'%'))->fetchAll();
  
  foreach ($result as $user) {
    $matches[$user->FILING_NUM] = check_plain($user->FILING_NAME);
  }
  print drupal_json_encode($matches);
  exit();
}
function wysos_autocomplete_mail_lookup($string = NULL) {
  $matches = array();
  $result = db_query_range("SELECT c.company_name, c.mail_action
  FROM wra_companies c
WHERE LOWER(c.company_name) LIKE LOWER(:company_name)
AND (c.scraped_ra_id=249 OR c.suite_id IS NOT NULL OR c.mail_action IS NOT NULL)
ORDER BY c.company_name", 0, 30,array(':company_name'=>db_like($string).'%'))->fetchAll(); 
  foreach ($result as $filing) {
    $matches[$filing->company_name] = check_plain($filing->company_name.' ['.$filing->mail_action.']');
  } 
  print drupal_json_encode($matches);
  exit();
}










