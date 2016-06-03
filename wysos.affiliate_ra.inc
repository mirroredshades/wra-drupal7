<?php 
// Affiliate RA

function wysos_affiliate_ra_summary() {
	drupal_set_title('Affiliate Registered Agents (RA)');
	$header = array( 'Company', 'User', 'Active', 'Delinquent', 'Inactive', 'Balance', 'Options');

	$result = db_query("SELECT ara.ra_name AS company_name, ara.ra_uid AS uid, ara.ra_username AS user_name, (SELECT COUNT(*) FROM wra_companies WHERE ra_id=ara.ra_id AND sos_status='Active') AS active, (SELECT COUNT(*) FROM wra_companies WHERE ra_id=ara.ra_id AND sos_status Like 'Delinquent%') AS delinquent, (SELECT COUNT(*) FROM wra_companies WHERE ra_id=ara.ra_id AND sos_status Like 'Inactive%') AS inactive, IFNULL((SELECT SUM(order_total) FROM uc_orders WHERE uid=ara.ra_uid) -
IFNULL((SELECT SUM(amount) FROM uc_payment_receipts WHERE order_id IN(SELECT order_id FROM uc_orders WHERE uid=ara.ra_uid)),0),0) AS user_balance, terms, ara.ra_id
			FROM wra_affiliate_ra ara
			WHERE ara.active=1")->fetchAll();
			
	$row = array();
	foreach($result as $raRow){
		$options = '<font size=-3>';
		if(!empty($raRow->terms)) {
			$options .= '<span class="qtip-link">
				<div class="qtip-tooltip">'. $raRow->terms.'</div>
				Terms
			  </span>';
		}
		$options .= ' '.l('This_Month','wysos/affiliate_ra/entities/'.$raRow->ra_id.'/'.date('n'));
		$options .= '</font>';
		$row[]  = array(
			'style' => 'background-color: LightGrey;',
			'data'  => 	array(
				array('data' => $raRow->company_name),
				array('data' => l($raRow->user_name,'wysos/user/'.$raRow->uid)),
				array('data' => l($raRow->active, 'wysos/affiliate_ra/entities/'.$raRow->ra_id.'/0/active')),
				array('data' => l($raRow->delinquent, 'wysos/affiliate_ra/entities/'.$raRow->ra_id.'/0/delinquent')),
				array('data' => l($raRow->inactive, 'wysos/affiliate_ra/entities/'.$raRow->ra_id.'/0/inactive')),
				array('data' => uc_currency_format($raRow->user_balance)),
				array('data' => $options)));
	} 
	//$output = theme('table', $header, $row);
	$output = array('#markup'=>theme('table',array('header'=>$header,'rows'=>$row)));
	return($output);			
}
function wysos_affiliate_ra_entities_form($form, &$form_state, $ra_id, $filing_month=NULL, $status=NULL) {
	//drupal_set_message('$form_state: '.print_r($form_state, TRUE));
	//$result = db_query("SELECT ra_id, ra_name FROM wra_affiliate_ra WHERE active=1 ORDER BY ra_name");
	
	$result = db_select('wra_affiliate_ra','war')
				->fields('war',array('ra_id','ra_name'))
				->condition('active','1','=')
				->execute()->fetchAll();
	
	foreach($result as $row) {
		$ra_name_options[$row->ra_id] = $row->ra_name; }

	$form_state['storage']['status'] = $status;
	$form_state['storage']['ra_id'] = $ra_id;
	$form_state['storage']['filing_month'] = $filing_month;
	if($filing_month == 12)
		$form_state['storage']['year'] = date('Y')-1;
	else
		$form_state['storage']['year'] = date('Y');
	$form['ra_name'] = array(
		'#type' => 'select',
		'#default_value' => arg(3),
		'#options' => $ra_name_options,
		'#prefix' => '<table><tr><td>RA</td><td>Month</td><td>Status</td></tr><tr><td>',
		'#suffix' => '</td>'
	);
	$form['month'] = array(
		'#type' => 'select',
		'#default_value' => arg(4),
		'#options' => array( 0 => 'All', 1 => 'January (1)', 2 => 'February (2)', 3 => 'March (3)', 4 => 'April (4)', 5 => 'May (5)',
			6 => 'June (6)', 7 => 'July (7)', 8 => 'August (8)', 9 => 'September (9)', 10 => 'October (10)', 11 => 'November (11)',
			12 => 'December (12)'),
		'#prefix' => '<td>',
		'#suffix' => '</td>'
	);
	$form['status'] = array(
		'#type' => 'select',
		'#default_value' => arg(5),
		'#options' => array(0=> 'All', 'active' => 'Active', 'delinquent' => 'Delinquent', 'inactive' => 'Inactive'),
		'#prefix' => '<td>',
		'#suffix' => '</td>'
//		'#attributes' => array('onchange' => 'window.location.href=this.form.URL.options[this.form.URL.selectedIndex].value'),
	);
	$form['filter'] = array(
		'#type' => 'submit',
		'#value' => 'Filter',
		'#prefix' => '<td>',
		'#suffix' => '</td></tr></table>',
//		'#attributes' => array('onclick' => 'changefilter(); return (true);'),
	);
	drupal_add_js("function changefilter() {
		var a = 'https://my.wyomingregisteredagent.com/wysos/affiliate_ra/entities/'+document.forms[0].ra_name.options[document.forms[0].ra_name.selectedIndex].value;
//		alert(a);
//		document.URL=a;
//		alert('Changed2');
	};", 'inline');


	//$result = db_query("SELECT * FROM wra_affiliate_ra WHERE ra_id=%d", $ra_id);
	//$raRow = db_fetch_array($result);
	 $raRow = db_select('wra_affiliate_ra','w')
				->fields('w')
				->condition("ra_id",$ra_id)
				->execute()->fetchAll();
	$form_state['storage']['ra_price'] = isset($raRow->ra_price)?$raRow->ra_price:'';
//drupal_set_message('ra_price: '.$form_state['storage']['ra_price']);	
	$form_state['storage']['ra_uid'] = isset($raRow->ra_uid)?$raRow->ra_uid:'';
	$ra_name = isset($raRow->ra_name)?$raRow->ra_name:'';
	$month_name = isset($month_name)?$month_name:'';
	if(empty($filing_month)) 
		$month_name = 'All Months';
	else
		$month_name = date("F", mktime(0, 0, 0, $filing_month, 10));
	if(empty($status))
		$status_name = "All";
	else
		$status_name = ucwords($status);
		
	drupal_set_title($status_name . ' entities for '.$ra_name.' ('.$month_name.')');
	
	$header = array(
		array(
			'data' => ' '),
		array(
			'data' => t('Filing ID'),
			'field' => 'next_ar_due'),
		array(
			'data' => t('Company Name'),
			'field' => 'filing_name',
			'sort' => 'asc' ),
		array(
			'data' => t('Filing Date'),
			'field' => 'filing_date'),
		array(
			'data' => t('Status'),
			'field' => 'sos_status'),
		array(
			'data' => t('Last Order'),
			'field' => 'name'),
		array(
			'data' => t('Options'),
			'field' => 'options'),
			//'options'
		);
	
	// Generate table
	require_once('wysos.util.inc');
	$ra_id = isset($raRow->ra_id)?$raRow->ra_id:'';
	$where = "WHERE c.ra_id=".$ra_id." ";
	if(!empty($status))
		$where .= "AND c.sos_status LIKE '".$status."%%' ";
	if(!empty($filing_month))
		$where .= "AND MONTH(c.filing_date) = ".$filing_month." ";
//	/*$order = "ORDER BY c.company_name ASC ";		
//	$query = sprintf("SELECT c.filing_num, LEFT(c.company_name,40) AS company_name, c.company_id, LEFT(c.sos_status,12) AS sos_status, c.eFNum, filing_date, 
//		MONTH(c.filing_date) AS filing_month, (SELECT MAX(o.order_id) 
//		FROM uc_orders o INNER JOIN uc_order_products p ON o.order_id=p.order_id 
//		WHERE o.uid=%d AND (o.data LIKE CONCAT('%%', c.filing_num, '%%') OR o.data LIKE CONCAT('%%', c.company_name, '%%'))) AS last_order
//		FROM wra_companies c
//		%s %s", $ra_id, $where, $order);
//	$result = db_query($query);*/
	
//	$query = sprintf("SELECT c.filing_num, LEFT(c.company_name,40) AS company_name, c.company_id, LEFT(c.sos_status,12) AS sos_status, c.eFNum, filing_date, 
//		MONTH(c.filing_date) AS filing_month, (SELECT MAX(o.order_id) 
//		FROM uc_orders o INNER JOIN uc_order_products p ON o.order_id=p.order_id 
//		WHERE o.uid=%d AND (o.data LIKE CONCAT('%%', c.filing_num, '%%') OR o.data LIKE CONCAT('%%', c.company_name, '%%'))) AS last_order
//		FROM wra_companies c %s",$ra_id, $where);
		
		
//		$result=db_query($query);
//		print_r($result);*/
		
		$result='';
		
	$form['table_header'] = array(
		'#type' => 'markup',
		'#value' => '<table><tr><th>&nbsp;</th><th>&nbsp;</th><th>Filing ID</th><th>Company Name</th><th>Filing Date</th><th>Status</th><th>Last Order</th><th>options</th></tr>');
		if($result){
			
			foreach($result as $raRow) {
				$i++;
				$options = '<font size=-3>';
				$options = l(t('Details'),'https://wyobiz.wy.gov/Business/FilingDetails.aspx?eFNum='.$raRow->eFNum, array('attributes' => array('target' => '_new1')));
				$options .= '</font>';
				$form['chk'.$raRow->company_id] = array(
					'#type' => 'checkbox', 
					'#name' => 'chk'.$raRow->company_id, 
					'#default_value' => '1',
					'#prefix' => '<tr><td>'.$i.'.</td><td>',
					'#suffix' => '</td><td>'.$raRow->filing_num.
						'</td><td>'.$raRow->company_name.
						'</td><td>'.$raRow->filing_date.
						'</td><td>'.$raRow->sos_status.
						'</td><td>'.$raRow->last_order.
						'</td><td>'.$options.'<td></tr>'
				);
			}
		}	
	$form['table_footer'] = array(
		'#type' => 'item',
		'#value' => '</table>' );
	$output = theme('table', array('rows'=>$row,'header'=>$header));
	$form['companies'] = array( '#type' => 'item', '#markup'=>$output);
	$form['action'] = array(
		'#type' => 'select',
		'#prefix' => '<table><tr><td>',
		'#suffix' => '</td>',
		'#options' => array( '' => '(Select Action)', 'invoice' => 'Invoice' ));
	$form['submit'] = array(
		'#type' => 'submit',
		'#prefix' => '<td>',
		'#suffix' => '</td></tr></table>',
		'#value' => 'Submit' );
	
	
	
	return($form);			

}

function wysos_affiliate_ra_entities_form_validate($form, &$form_state)
{
	
}
function wysos_affiliate_ra_entities_form_submit(&$form, &$form_state)
{		
	switch($form_state['clicked_button']['#value']) 
	{
		case 'Filter' :
			drupal_redirect_form($form, 'wysos/affiliate_ra/entities/'.$form_state['values']['ra_name'].'/'.$form_state['values']['month'].'/'.$form_state['values']['status']);
			break;
			
		case 'Submit' :
			switch($form_state['values']['action'])
			{
				case 'invoice' :
					include_once('wysos.util.inc');
					$order_id = wysos_util_create_order_from_uid($form_state['storage']['ra_uid']);
					if(empty($form_state['storage']['filing_month'])) 
						$month_name = 'All Months';
					else
						$month_name = date("F", mktime(0, 0, 0, $form_state['storage']['filing_month'], 10));
						
					$where = "WHERE c.ra_id=".$form_state['storage']['ra_id']." ";
					if(!empty($form_state['storage']['status']))
						$where .= "AND c.sos_status LIKE '".$form_state['storage']['status']."%' ";
					if(!empty($form_state['storage']['filing_month']))
						$where .= "AND MONTH(c.filing_date) = ".$form_state['storage']['filing_month']." ";
					$query = sprintf("SELECT c.company_id, c.company_name, c.filing_num 
						FROM wra_companies c
						%s", $where);						
				
					$result = db_query($query);
					while ($raRow = db_fetch_array($result)) {
						if($form_state['values']['chk'.$raRow['company_id']]) {
							$company_list .= $raRow['filing_num'] . ' '. $raRow['company_name']."; ";
							$i++;
						}
					} 
					$price = $i * $form_state['storage']['ra_price'];
					if(	$form_state['storage']['ra_id'] == 464 ) { /* US Corporation Agents */
						$data = array('attributes' => array(
							'Month' => array(date("F", strtotime("-1 month"))),
							'Year' => array(date("Y", strtotime("-1 month"))),
							'Company List' => $company_list
							));
						db_query("INSERT INTO uc_order_products (order_id, nid, title, model, qty, price, data)
							VALUES(%d, 1880, 'Affiliate Registered Agent Service ($0.75 per entity per month)', 'ARA', 1, %f, '%s')", $order_id, $price, serialize($data));
						
					} else {
						$data = array('attributes' => array(
							'Month' => array(date("F", mktime(0, 0, 0, $form_state['storage']['filing_month'], 10))),
							'Year' => array($form_state['storage']['year']),
							'Company List' => $company_list
							));
						db_query("INSERT INTO uc_order_products (order_id, nid, title, model, qty, price, data)
							VALUES(%d, 1880, 'Affiliate Registered Agent Service (Annual)', 'ARA', 1, %f, '%s')", $order_id, $price, serialize($data));
					}
					
					db_query("UPDATE uc_orders SET order_total=%f WHERE order_id=%d", $price, $order_id);
					drupal_redirect_form($form, 'admin/store/orders/'.$order_id.'/invoice');
					break;
			};
			break;
	}
	
}