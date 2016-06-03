<?php
function wysos_billing_make_renewals_form($form, &$form_state){
	$form['renewal_month'] = array(
		'#title' => t('Renewal Month'),
		'#type' => 'select',
		'#options' => array(1=>'Jan (1)', 2=>'Feb (2)', 3=>'Mar (3)', 4=>'Apr (4)', 5=>'May (5)',  6=>'Jun (6)', 
			7=>'Jul (7)', 8=>'Aug (8)', 9=>'Sep (9)', 10=>'Oct (10)', 11=>'Nov (11)', 12=>'Dec (12)'),
        '#default_value' => date('n'),
		'#required' => TRUE
	);
	$year = date('Y');
		$form['renewal_year'] = array(
			'#title' => t('Renewal Year'),
			'#type' => 'select',
			'#options' => array($year=>$year, $year+1=>$year+1),
			'#default_value' => $year,
			'#required' => TRUE
		);
		$form['submit'] = array (
			'#type' => 'submit',
			'#value' => t('Submit')
		);
	return $form; 
}
function wysos_billing_make_renewals_form_submit($form, &$form_state) {
	
	$month = $form_state['values']['renewal_month'];
	$year = $form_state['values']['renewal_year'];
	
	
	$result = db_update('WySosUserFiling')
			->fields(array(
				'nominee_product' => 'NULL',
				'nominee_price' => 'NULL',
			))->condition('nominee_product','')->execute();
			
	$result = db_update('WySosUserFiling')
			->fields(array(
				'nominee_product' => 'NULL',
				'nominee_price' => 'NULL',
			))->condition('nominee_price',0)->execute();
	
	$result = db_update('WySosUserFiling')
			->fields(array(
				'mail_suite' => 'NULL',
			))->condition('mail_suite',0)->execute();	


$results = db_query("SELECT c.company_name, c.filing_type, c.formation_locale, c.uid, c.filing_num, c.filing_id, c.company_id, c.sos_status, c.last_ar_year, FROM_UNIXTIME(c.next_ar_due), '".$year."',  '".$year."-".$month."-01' as due_date, 
DATE_FORMAT('".$year."-".$month."-01','%%M %%e, %%Y') AS tdue_date,
If(c.last_ar_year<".$year." Or IsNull(c.last_ar_year),If(c.filing_type='NonProfit Corporation',25,If(c.filing_type='Statutory Trust',100,50)),Null) AS ar_price, u.name, u.mail, a.first_name, a.last_name, a.company, a.street1, a.street2, a.city, z.zone_code, a.postal_code, If(co.country_name='United States',NULL,co.country_name), 
c.nominee_sku, c.nominee_price,
c.ra_price, s.suite_id, s.billing_period, s.price,
If(c.last_ar_year<".$year." Or IsNull(c.last_ar_year),'Annual Report',NULL) AS Detail1, If(c.last_ar_year<".$year." Or IsNull(c.last_ar_year),
If(c.filing_type='NonProfit Corporation','\$25.00','\$50.00'),Null) AS Detail1Price, If(IsNull(c.ra_price),Null,'Registered Agent') AS Detail2,
CONCAT('\$',Format(c.ra_price,2)) AS Detail2Price, If(IsNull(c.nominee_price),Null,'Nominee Privacy') AS Detail3, CONCAT('\$',FORMAT(c.nominee_price,2)) AS Detail3Price, 
If(Not IsNull(s.price) And s.billing_period='year','Virtual Office',Null) AS Detail4, 
CONCAT('\$',If(Not IsNull(s.price) And s.billing_period='year',s.price,Null)) AS Detail4Price,
If(IsNull(c.nominee_price),0,c.nominee_price)+If(IsNull(c.ra_price),0,c.ra_price)+If(s.billing_period='year' And Not IsNull(s.price),s.price,0)+If(c.last_ar_year< ".$year."
Or IsNull(c.last_ar_year),If(c.filing_type='NonProfit Corporation',25,50),0) AS total_due, 		
CONCAT('\$',Format(If(IsNull(c.nominee_price),0,c.nominee_price)+If(IsNull(c.ra_price),0,c.ra_price)+If(s.billing_period='year' And Not IsNull(s.price),s.price,0)+
If(c.last_ar_year<".$year." Or IsNull(c.last_ar_year),If(c.filing_type='NonProfit Corporation',25,50),0),2)) AS ttotal_due

FROM wra_companies c
LEFT JOIN users u ON c.uid=u.uid
LEFT JOIN uc_addresses_defaults ad ON c.uid=ad.uid
LEFT JOIN uc_addresses a ON a.aid = ad.aid
LEFT JOIN uc_countries co ON a.country=co.country_id
LEFT JOIN uc_zones z ON a.zone=z.zone_id
LEFT JOIN wra_suites s ON c.suite_id=s.suite_id
WHERE Month(c.filing_date)=".$month." AND Year(c.filing_date)<".$year." AND (c.sos_status='Active') And (c.filing_type IN('Profit Corporation','Limited Liability Company','NonProfit Corporation','Limited Partnership','Statutory Trust','Registered Limited Liability Partnership','General Partnership')) AND c.ra_id=249 AND c.uid NOT IN(5)")->fetchAll();

foreach($results as $result){
	$ins = db_insert('wra_renewals')
		->fields(array(
			'filing_name' => $result->company_name,
			'FILING_TYPE' => $result->filing_type,
			'FORMATION_LOCALE' => $result->formation_locale,
			'uid' => $result->uid,
			'FILING_NUM' => $result->filing_num,
			'FILING_ID' => $result->filing_id,
			'company_id' => $result->company_id,
			'filing_status' => $result->sos_status,
			'last_ar' => $result->last_ar_year,
			'next_ar_due' => $result->FROM_UNIXTIME(c.next_ar_due),
			'ar_year' => $year,
			'due_date' => $result->due_date,
			'tdue_date' => $result->tdue_date,
			'ar_price' => $result->ar_price,
			'user_name' => $result->name,
			'email' => $result->mail,
			'first_name' => $result->first_name,
			'last_name' => $result->last_name,
			'company' => $result->company,
			'street1' => $result->street1,
			'street2' => $result->street2,
			'city' => $result->city,
			'state' => $result->zone_code,
			'postal_code' => $result->postal_code,
			//'country' => $result->If(co.country_name='United States',NULL,co.country_name),
			'country' => $result->co.country_name,
			'nominee_product' => $result->nominee_sku,
			'nominee_price' => $result->nominee_price,
			'ra_price' => $result->ra_price,
			'mail_suite' => $result->suite_id,
			'billing_period' => $result->billing_period,
			'vo_price' => $result->price,
			'Detail1' => $result->Detail1,
			'Detail1Price' => $result->Detail1Price,
			'Detail2' => $result->Detail2,
			'Detail2Price' => $result->Detail2Price,
			'Detail3' => $result->Detail3,
			'Detail3Price' => $result->Detail3Price,
			'Detail4' => $result->Detail4,
			'Detail4Price' => $result->Detail4Price,
			'total_due' => $result->total_due,
			'ttotal_due' => $result->ttotal_due,
		))->execute();

}

	if(!$results) {
		drupal_set_message("Query error: ");
		return;
	}
		
	//$query = "UPDATE `wra_renewals` SET invoice_no = CONCAT('R',LPAD(renewal_id,6,'0')) WHERE invoice_no IS NULL;";
	
	//$result = db_query($query);
	
	$result = db_result('wra_renewals')
			->fields(array(
				'invoice_no' => CONCAT('R',LPAD(renewal_id,6,'0')),
			))->condition('invoice_no','NULL')->execute();
	
	if(!$result) {
		drupal_set_message("Query error");
		return;
	}
	
	// Set Shelf Companies and kvogt companies to paid
	//$result = db_query("UPDATE wra_renewals SET payment_date=NOW(), payment_amount=0, status='Autopay (shelf)' WHERE due_date='%d-%d-01' AND uid IN(136,3)", $year, $month);
	$ucheck[] =array('136'=>'136','3'=>'3') ;
	$result = db_update('wra_renewals')
			->fields(array(
				'payment_date' => REQUEST_TIME,
				'payment_amount' => 0,
				'status' => 'Autopay (shelf)',
			))->condition('due_date', '".$year."-".$month."-01')->condition('uid',$ucheck,'IN')->execute();
	
	if(!$result) {
		drupal_set_message("Query error");
		return;
	}

}
