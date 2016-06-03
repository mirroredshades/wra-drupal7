<?php 
function wysos_shelf_list() {
	// Table Header
	$headers = array(
		array(
			'data' => t('Filing Num'),
			'field' => 'filing_num'),
		array(
			'data' => t('Company Name'),
			'field' => 'company_name'),
		array(
			'data' => t('Formation Date'),
			'field' => 'filing_date',
			'sort' => 'desc' ),
		array(
			'data' => t('Status'),
			'field' => 'sos_status'),
		array(
			'data' => t('Price'),
			'field' => 'sell_price'),
		array(
			'data' => t('Options'),
			'field' => 'options'),
		);
		
	// For Sale and In Stock
	//$output = array();
	$output = '<h2>For Sale</h2>';
	$output .= '<h3>Profit Corporations</h3>';
	
	$result = db_select('wra_companies', 'c');
	$result->join('uc_products', 'p', "p.model=CONCAT('SC',c.filing_num)");
	$result->leftJoin('uc_product_stock', 's', 's.nid= p.nid');
	$result->condition('c.uid', 136, '=');
	$result->condition('c.scraped_ra_id', 249, '=');
	$result->condition('s.stock', 0, '>=');
	$result->condition('c.filing_type', 'Profit Corporation', '=');
	$result->fields('c', array('company_id', 'company_name', 'filing_num', 'eFNum', 'sos_status', 'filing_date'));
	$result->fields('p', array('sell_price', 'nid'));
	$result->orderBy('p.sell_price', 'DESC');
	$dbrows = $result->execute()->fetchAll();
	
	foreach($dbrows as $dbrow) {
		$options = '<font size=-3>';
		$options .= l('Product','node/'.$dbrow->nid);
		$options .= ' '.l('Company', 'wysos/company/browse/'.$dbrow->company_id);
		if($dbrow->sos_status == 'Delinquent - Tax' || $dbrow->sos_status == 'Active') {
		// File AR
			$options .= ' ' . l(t('File AR'),'https://wyobiz.wy.gov/Business/ARWizard.aspx?eFNum=' . $dbrow->eFNum, array('attributes' => array('target' => '_new2')));
		} elseif ($row->Status == 'Inactive - Administratively Dissolved (Tax)') {
			$options .= ' ' . l(t('Reinstate'),'https://my.wyomingregisteredagent.com/wysos/reinstate/' . $dbrow->filing_num, array('attributes' => array('target' => '_new2')));
		}
		$options .= '</font>';
  		$rows[] = array(l($dbrow->filing_num,'https://wyobiz.wy.gov/Business/FilingDetails.aspx?eFNum='.$dbrow->eFNum), 
			$dbrow->company_name,
			$dbrow->filing_date, 
			substr($dbrow->sos_status,0,10), 
			uc_currency_format($dbrow->sell_price), 
			$options);
	}
	
	//$output .= theme_table($headers,$rows).'<p>';
	$output .=  theme('table',array('header'=>$headers, 'rows'=>$rows));
	
	
	$output .= '<h3>LLCs</h3>';
	$rows = array();
	
	$result = db_select('wra_companies', 'c');
	$result->join('uc_products','p', "CONCAT('SC',c.filing_num)=p.model");
	$result->leftJoin('uc_product_stock', 's', 's.nid = p.nid');
	$result->condition('c.uid',136,'=');
	$result->condition('c.scraped_ra_id',249,'=');
	$result->condition('s.stock', 0, '>=');
	$result->condition('c.filing_type', 'Limited Liability Company', 'LIKE');
	$result->fields('c',array('company_id','company_name','filing_num','eFNum','sos_status','filing_date'));
	$result->fields('p',array('sell_price','nid'));
	$result->orderBy('p.sell_price','DESC');
	$dbrows = $result->execute()->fetchAll();
	
	
	foreach($dbrows as $dbrow) {
		$options = '<font size=-3>';
		$options .= l('Product','node/'.$dbrow->nid);
		$options .= ' '.l('Company', 'wysos/company/browse/'.$dbrow->company_id);
		if($dbrow->sos_status == 'Delinquent - Tax' || $dbrow->sos_status == 'Active') {
		// File AR
			$options .= ' ' . l(t('File AR'),'https://wyobiz.wy.gov/Business/ARWizard.aspx?eFNum=' . $dbrow->eFNum, array('attributes' => array('target' => '_new2')));
		} elseif ($row->Status == 'Inactive - Administratively Dissolved (Tax)') {
			$options .= ' ' . l(t('Reinstate'),'https://my.wyomingregisteredagent.com/wysos/reinstate/' . $dbrow->filing_num, array('attributes' => array('target' => '_new2')));
		}
		$options .= '</font>';
  		$rows[] = array(l($dbrow->filing_num,'https://wyobiz.wy.gov/Business/FilingDetails.aspx?eFNum='.$dbrow->eFNum), 
			$dbrow->company_name,
			$dbrow->filing_date, 
			substr($dbrow->sos_status,0,10), 
			uc_currency_format($dbrow->sell_price), 
			$options);
	}
	
	//$output .= theme_table($headers,$rows).'<p>';
	$output .= theme('table', array('header'=>$headers,'rows'=>$rows));
	// In Storage
	$headers = array(
		array(
			'data' => t('Filing Num'),
			'field' => 'filing_num'),
		array(
			'data' => t('Company Name'),
			'field' => 'company_name'),
		array(
			'data' => t('Formation Date'),
			'field' => 'filing_date',
			'sort' => 'desc' ),
		array(
			'data' => t('Status'),
			'field' => 'sos_status'),

			'Options'
		);
	$rows = array();
	$output .= '<h2>In Storage</h2>';
	
	$result = db_select('wra_companies', 'c');
	$result->leftJoin('uc_products', 'p', "p.model = CONCAT('SC',c.filing_num)");
	$result->leftJoin('uc_product_stock', 's', 's.nid = p.nid');
	$result->condition('c.uid',136,'=');
	$result->condition('c.scraped_ra_id',249,'=');
	//$result->condition('s.stock',0,'=');
	$result->isNull('s.stock');
	//$result->where("s.stock = 0");
	$result->fields('c',array('company_id','company_name','filing_num','eFNum','sos_status','filing_date'));
	$dbrows = $result->execute()->fetchAll();
	
	foreach($dbrows as $dbrow) {
		$options = '<font size=-3>';
		$options .= l('Add_Product','wysos/shelf/add_product/'.$dbrow->filing_num);
		$options .= ' '.l('Company', 'wysos/company/browse/'.$dbrow->company_id);
		if($dbrow->sos_status == 'Delinquent - Tax' || $dbrow->sos_status == 'Active') {
		// File AR
			$options .= ' ' . l(t('File AR'),'https://wyobiz.wy.gov/Business/ARWizard.aspx?eFNum=' . $dbrow->eFNum, array('attributes' => array('target' => '_new2')));
		} elseif ($row->Status == 'Inactive - Administratively Dissolved (Tax)') {
			$options .= ' ' . l(t('Reinstate'),'https://my.wyomingregisteredagent.com/wysos/reinstate/' . $dbrow->filing_num, array('attributes' => array('target' => '_new2')));
		}
		$options .= '</font>';
  		$rows[] = array(l($dbrow->filing_num,'https://wyobiz.wy.gov/Business/FilingDetails.aspx?eFNum='.$dbrow->eFNum), 
			$dbrow->company_name,
			$dbrow->filing_date, 
			substr($dbrow->sos_status,0,10), 
			$options);
	}
	
	//$output .= theme_table($headers,$rows).'<p>';
	$output .= theme('table', array('header'=>$headers, 'rows'=>$rows));
	// In Storage
	$headers = array(
		array(
			'data' => t('Filing Num'),
			'field' => 'filing_num'),
		array(
			'data' => t('Company Name'),
			'field' => 'company_name'),
		array(
			'data' => t('Formation Date'),
			'field' => 'filing_date',
			'sort' => 'desc' ),
		array(
			'data' => t('Status'),
			'field' => 'sos_status'),
		);
	$rows = array();
	$output .= "<h2>Unassigned</h2>";
	
	$result = db_select('wra_companies', 'c');
	$result->join('uc_products', 'p', "CONCAT('SC',c.filing_num) = p.model");
	$result->leftJoin('uc_product_stock', 's', 'p.nid=s.nid');
	$result->condition('c.uid',136,'=');
	$result->condition('c.scraped_ra_id',249,'=');
	$result->condition('s.stock', 1, '<');
	$result->fields('c',array('company_id','company_name','filing_num','eFNum','sos_status','filing_date'));
	//$result->fields
	$dbrows = $result->execute()->fetchAll();
	
	foreach($dbrows as $dbrow) {
		$options = '<font size=-3>';
		$options .= l('Assign', 'wysos/company/browse/'.$dbrow->company_id);
		$options .= ' '.l('Company', 'wysos/company/browse/'.$dbrow->company_id);
		if($dbrow->sos_status == 'Delinquent - Tax' || $dbrow->sos_tatus == 'Active') {
		// File AR
			$options .= ' ' . l(t('File AR'),'https://wyobiz.wy.gov/Business/ARWizard.aspx?eFNum=' . $dbrow->eFNum, array('attributes' => array('target' => '_new2')));
		} elseif ($row->Status == 'Inactive - Administratively Dissolved (Tax)') {
			$options .= ' ' . l(t('Reinstate'),'https://my.wyomingregisteredagent.com/wysos/reinstate/' . $dbrow->filing_num, array('attributes' => array('target' => '_new2')));
		}
		$options .= '</font>';
  		$rows[] = array(l($dbrow->filing_num,'https://wyobiz.wy.gov/Business/FilingDetails.aspx?eFNum='.$dbrow->eFNum), 
			$dbrow->company_name,
			$dbrow->filing_date, 
			substr($dbrow->sos_status,0,10), 
			$options);
	}
	
	//$output .= theme_table($headers,$rows).'<p>';
	$output .= theme('table',array('header'=>$headers,'rows'=>$rows));
	
	return($output);
}

function wysos_shelf_for_sale() {
	$headers = array(
		array(
			'data' => t('Filing Num'),
			'field' => 'filing_num'),
		array(
			'data' => t('Company Name'),
			'field' => 'company_name'),
		array(
			'data' => t('Formation Date'),
			'field' => 'filing_date',
			'sort' => 'desc' ),
		array(
			'data' => t('Status'),
			'field' => 'sos_status'),
		array(
			'data' => t('Price'),
			'field' => 'sell_price'),
			'Options'
		);

	// Generate table	
	$result = db_select('wra_companies', 'wc');
	$result->join('uc_products', 'up', "up.model = CONCAT('SC',wc.filing_num)");
	$result->leftJoin('uc_product_stock', 's', 'up.nid = s.nid');
	$db_or = db_or();
	$db_and = db_and();
	$db_and->condition('wc.uid', 136, '=');
	$db_and->condition('wc.scraped_ra_id', 249, '=');
	$db_and->condition('s.stock',0,'>=');
	$db_or->condition($db_and);
	$result->condition($db_or);
	$result->fields('wc',array('company_id','company_name','filing_num','eFNum','sos_status','filing_date'));
	$result->fields('up', array('sell_price','nid'));
	$dbrows = $result->execute()->fetchAll();
	
	foreach ($dbrows as $dbrow) {
		$options = '<font size=-3>';
		if(empty($dbrow->sell_price)) {
			$options .= l('Add_Product','wysos/shelf/add_product/'.$dbrow->filing_num);
		} else {
			$options .= l('Product','node/'.$dbrow->nid);
		}
		$options .= ' '.l('Company', 'wysos/company/browse/'.$dbrow->company_id);
		$options .= '</font>';
  		$rows[] = array(l($dbrow->filing_num,'https://wyobiz.wy.gov/Business/FilingDetails.aspx?eFNum='.$dbrow->eFNum), 
			$dbrow->company_name,
			$dbrow->filing_date, 
			substr($dbrow->sos_status,0,10), 
			uc_currency_format($dbrow->sell_price), 
			$options);	
	}
	$output = array('#markup'=>theme('table', array('header'=>$headers,'rows'=>$rows)));
	return $output;
}

function wysos_shelf_add_product($filing_num) {
	global $user;

	// Get Number of Days Difference 
	$rows = db_query("SELECT DATEDIFF(CURDATE(),filing_date) AS age_days, CASE WHEN DATEDIFF(CURDATE(),filing_date) BETWEEN 0 AND 365 THEN 500 
WHEN DATEDIFF(CURDATE(),filing_date) BETWEEN 366 AND 730 THEN 550
WHEN DATEDIFF(CURDATE(),filing_date) > 730 THEN 550+((FLOOR(DATEDIFF(CURDATE(),filing_date) / 90)-7)*80)
END AS price, company_name, filing_date 
FROM wra_companies WHERE filing_num= '".$filing_num."'")->fetchObject();
//print_r($rows);

	if($rows){
	$newNode = (object) NULL;
	$newNode->type = 'product';
	$newNode->title = "Shelf Company-".$rows->company_name;
	$newNode->uid = $user->uid;
	$newNode->created = strtotime("now");
	$newNode->changed = strtotime("now");
	$newNode->status = 1;
	$newNode->comment = 0;
	$newNode->promote = 0;
	$newNode->moderate = 0;
	$newNode->sticky = 0;
	$newNode->language = 'und';
	node_save($newNode);
}
	$nid = db_query("SELECT MAX(nid) FROM node")->fetchField();

	//$body = "Wyoming shelf company ".$rows->company_name." incorporated ".$rows->filing_date;
	
	$result = db_update('uc_products')
				->fields(array(
				'model'=> 'SC'.$filing_num,
				'cost' => 100,
				'sell_price'=> isset($rows->price)?$rows->price:'0',
				))
				->condition('nid',$nid,'=')
				->execute();
	
	//db_query("INSERT INTO uc_products (nid,vid,model, list_price, cost, sell_price) VALUES (%d, %d, 'SC%s', %f, 100, %f)", $nid, $vid, $filing_num, $price, //$rows['price']);

	// Insert Product Stock
	//db_query("INSERT INTO uc_product_stock (sku, nid, active, stock) VALUES ('SC%s', %d, 1, 1)", $filing_num, $nid);
	
	$result = db_update('uc_product_stock')
				->fields(array(
				'sku'=> 'SC'.$filing_num,
				'active' => 1,
				'stock'=> 1,
				))
				->condition('nid',$nid,'=')
				->execute();
	
	drupal_goto('/');
}


