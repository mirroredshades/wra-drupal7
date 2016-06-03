<?php

// Batch Operations

function wysos_batch_list() {
	// List Latest Batches
	drupal_set_title('Batches from last 60 days');
/*
	// Monthly Virtual Offices
	$header = array( 'Batch ID', 'Title', 'Created', 'Orders', 'Options');
	$orderHeader = array(t('Order No.'),t('Date'),t('Order Status'),t('Total'),t('Balance Due'), t('Order Options'));
	
	$result = db_query("SELECT b.batch_id, b.title, b.created, b.anniv_day, b.anniv_month, b.anniv_year, 
			(SELECT COUNT(*) FROM wra_batch_order bo WHERE bo.batch_id=b.batch_id) AS num_orders
			FROM wra_batch b WHERE b.created > UNIX_TIMESTAMP()-2764800 AND finished != 1 ORDER BY b.created DESC"); 

	if (!$result) {
		die('Invalid query');
	}  

	while ( $rowBatch = db_fetch_array($result) ) {
		// Set Options	
		$options = '<font size=-3>';
		$options .= '</font>';
		
		$form[$rowBatch['batch_id']] = array(
			'#title' => $rowBatch['title'].' ('.$rowBatch['num_orders'].')',
			'#prefix' => '<div id="batch'.sprintf($rowBatch['batch_id'], '%010d').'">',
			'#suffix' => '</div>',
			'#type' => 'fieldset',
			'#weight' => $rowBatch['batch_id'],
			'#collapsible' => TRUE, 
			'#collapsed' => TRUE
		);
		$orderHeader = array(t('Invoice'),t('Suite'),t('Company'),t('User'),t('Status'),t('Total'),t('Due'),t('Pay By'),t('User Balance'),t('Options'));
		$orderRow = array();

		$result2 = db_query("SELECT o.order_id, o.order_status, o.order_total, o.order_total-IFNULL((SELECT SUM(r.amount) FROM uc_payment_receipts r WHERE r.order_id=o.order_id),0) AS balance_due, s.suite_id, s.suite_no, c.company_name, s.uid, u.name AS user_name, s.status as suite_status, s.chargify_subscription_id,
			ru.next_charge, s.pay_by, (SELECT SUM(order_total) FROM uc_orders WHERE uid=u.uid) -
IFNULL((SELECT SUM(amount) FROM uc_payment_receipts WHERE order_id IN(SELECT order_id FROM uc_orders WHERE uid=u.uid)),0) AS user_balance,
wo.OrderId AS web_order_id
				FROM wra_batch_order bo
				INNER JOIN uc_orders o ON o.order_id = bo.order_id
				LEFT JOIN wra_suites s ON bo.suite_id = s.suite_id
				INNER JOIN users u ON s.uid=u.uid
				LEFT JOIN wra_companies c ON s.company_id=c.company_id
				LEFT JOIN uc_recurring_users ru ON s.orig_recur_order_id = ru.order_id
				LEFT JOIN fed_www.orders wo ON s.orig_order_id = wo.uc_orderid
				WHERE bo.batch_id=%d
				ORDER BY o.order_id ASC", $rowBatch['batch_id']);
		
		$row = array(array());
		while ($orderRow = db_fetch_array($result2)) {
			$pay_by = $orderRow['pay_by'];
			if($orderRow['next_charge'] > mktime(0,0,0,$rowBatch['anniv_month'],$rowBatch['anniv_day'],$rowBatch['anniv_year']))
				$pay_by = date('m-d-Y',$orderRow['next_charge']);
			$options = '<font size=-3>';
			if($orderRow['balance_due'] > 0.0) {
				$options .= l('Email_Signup', 'wysos/virtualoffice/email_signup/'.$orderRow['suite_id'], array('attributes' => array('target' => '_email'))).' ';

			};
			if(!empty($orderRow['web_order_id']))
				$options .= l('Web_Order', 'wysos/suite/web_order/' . $orderRow['web_order_id'], array('attributes' => array('target' => '_credit'))).' '; 

			if($orderRow['suite_status'] == 'active')
				$options .= l('Suspend', 'wysos/suite/suspend/'.$orderRow['suite_id']).' ';

			$options .= '</font>';
			$row[]  = array(
				'style' => 'background-color: LightGrey;',
				'data'  => 	array(
					array('data' => l($orderRow['order_id'], 'admin/store/orders/'.$orderRow['order_id'], array('attributes' => array('target' => '_orders')))),
					array('data' => l($orderRow['suite_no'], 'wysos/suite/browse/'.$orderRow['suite_id'], array('attributes' => array('target' => '_suites')))),
					array('data' => $orderRow['company_name']),
					array('data' => l($orderRow['user_name'],'wysos/user/'.$orderRow['uid'], array('attributes' => array('target' => '_user')))),
					array('data' => $orderRow['order_status']),
					array('data' => uc_currency_format($orderRow['order_total'])),
					array('data' => uc_currency_format($orderRow['balance_due'])),
					array('data' => $pay_by),
					array('data' => uc_currency_format($orderRow['user_balance'])),
					array('data' => $options)));
		} 
			$row[]  = array(
				'style' => 'background-color: LightGrey;',
				'data'  => 	array(
					array('data' => l("Get PDF", 'wysos/batch/pdf/'.$rowBatch['batch_id'])),
					array(),
					array(),
					array(),
					array(),
					array(),
					array(),
					array(),
					array()));
		$form[$rowBatch['batch_id']]['list'] = array('#type' => 'markup', '#value' => theme('table', $orderHeader, $row)); 
	} 
				
	return(drupal_render($form));
	*/
}

function wysos_batch_pdf($batch_id) {
	if(!include_once("sites/all/modules/ubercart/uc_order/uc_order.admin.inc"))
		drupal_set_message('include error');
		
	$result = db_query("SELECT order_id FROM wra_batch_order WHERE batch_id=%d ORDER BY order_id", $batch_id); 
	while($order = db_fetch_array($result)) {
		$order = uc_order_load($order['order_id']);
		$output .= "<div style='page-break-after:always'>";
		$output .= uc_order_invoice($order, 'view');
		$output .= "</div>";
	}
	
	require_once('dompdf/dompdf_config.inc.php');
	$dompdf = new DOMPDF();
	$dompdf->load_html($output);
	$dompdf->render();
	$dompdf->stream("sample.pdf");

}