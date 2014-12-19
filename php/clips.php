<?php defined("BASEPATH") or exit("No direct script access allowed");

function clips_str_match($str, $pattern) {
	return !!preg_match('/'.$pattern.'/', $str);
}

function clips_get_property($obj, $property) {
	if(is_array($obj) && isset($obj[$property])) {
		return $obj[$property];
	}

	if(is_object($obj) && isset($obj->$property)) {
		return $obj->$property;
	}
	return null;
}
/**
 *  The clips extension engine and execution context
 */
class Clips {

	/**
	 * The clips execution context
	 */
	public static $context;

	public function __construct() {
		if(!Clips::$context) {
			Clips::$context = array();
			clips_init(Clips::$context);
		}
		$this->clear();
	}

	private function _init_base_support() {
		$this->defineClasses();
		$this->defineMethods();
		if(function_exists('get_instance')) {
			$this->ci = get_instance(); // Add the ci object to the context, if function get_instance is exists
		}
		$path = dirname(__FILE__).'/rules/clips.rules'; // Load the default functions
	}

	private function defineMethods() {
		$this->command('(deffunction ci_load (?file) (php_call "clips_load_rule" ?file))'); // Define the ci_load function
	}

	private function defineClasses() {
		if(!$this->classExists('PHP_OBJECT'))
			$this->command($this->defineClass('PHP_OBJECT', 
				array('USER', 'OBJECT'), false, array()));
	}

	private function translate($var) {
		if($var === null)
			return 'nil';

		switch(gettype($var)) {
		case 'string':
			return '"'.$var.'"';
		case 'boolean':
			return $var? 'TRUE' : 'FALSE';
		case 'array':
		case 'object':
			// For array and object, let's make them multiple values
			$ret = array();
			foreach($var as $key => $value) {
				$ret []= $this->translate($value);
			}
			return implode(' ', $ret);
		}
		return $var;
	}

	public function template($class) {
		if(is_array($class)) {
			foreach($class as $c) {
				$this->template($c);
			}
			return true;
		}
		if(!$this->templateExists($class)) {
			$this->command($this->defineTemplate($class));
		}
	}

	public function defineTemplate($class) {
		if(is_string($class) && class_exists($class)) {
			$ret = array();
			$ret []= '(deftemplate '.$class;
			foreach(get_class_vars($class) as $slot => $v) {
				if(preg_match('/s$/', $slot))
					$ret []= '(multislot '.$slot.')';
				else
					$ret []= '(slot '.$slot.')';
			} return implode(' ', $ret).')';
		}
		return null;
	}

	public function defineSlot($name, $type = 'slot', $default = null, $constraints = array()) {
		$slot = array();
		$slot []= '('.$type; // Add the slot define
		$slot []= $name;
		if($default !== null) {
			$slot []= '(default '.$default.')';
		}

		foreach($constraints as $c) {
			if(isset($c['type'])) {
				switch($c['type']) {
				case 'range':
				case 'cardinality': // For range and cardinality, we use 2 parameters
					$slot []= '('.$c['type'].' '.$c['begin'].' '.$c['end'].')'; // Default is (type value)
					break;
				default:
					$slot []= '('.$c['type'].' '.$c['value'].')'; // Default is (type value)
				}
			}
		}
		return implode(' ', $slot).')';
	}

	public function defineClass($class, $parents, $abstract = false, $slots = null, $methods = null) {
		$ret = array();
		$ret []= '(defclass '.$class;
		$p = $parents;
		if(is_array($parents)) {
			$p = implode(' ', $parents);
		}

		$ret []= '(is-a '.$p.')';

		if($abstract) {
			$ret []= '(role abstract)';
		}
		else {
			$ret []= '(role concrete)';
		}

		if($slots != null) {
			foreach($slots as $slot) {
				if(is_string($slot))
					$ret []= $this->defineSlot($slot);
				else
					$ret []= $this->defineSlot(
						$slot['name'], 
						get_default($slot, 'type', 'slot'),
						get_default($slot, 'default'),
						get_default($slot, 'constraints', array())
					);
			}
		}

		return implode(' ', $ret).')';
	}

	public function reset() {
		$this->command('(reset)');
	}

	public function clear() {
		foreach(Clips::$context as $key => $value) {
			unset(Clips::$context[$key]);
		}
		$this->command('(clear)');
		$this->_init_base_support();
	}

	public function facts() {
		$this->command('(facts)');
	}

	public function run() {
		$this->command('(run)');
	}

	public function __get($key) {
		if(isset(Clips::$context[$key])) {
			return Clips::$context[$key];
		}
		return $this->$key;
	}

	public function defineInstance($name, $class = 'PHP_OBJECT', $args = array()) {
		$ret = array();
		$ret []= '(make-instance';
		$ret []= $name;
		$ret []= 'of';
		$ret []= $class;
		foreach($args as $key => $value) {
			$ret []= '('.$key;
			$ret []= $value;
			$ret []= ')';
		}
		return implode(' ', $ret).')';
	}

	public function __set($key, $value) {
		Clips::$context[$key] = $value;
		if(!$this->instanceExists($key)) {
			$this->command($this->defineInstance($key));
		}
	}

	/**
	 * Start the clips in console mode
	 */
	public function console() {
		$line = readline('pclips$ ')."\n";
		while(true) {
			if(clips_is_command_complete($line)) {
				$this->command($line);
				readline_add_history($line);
				$line = readline('pclips$ ')."\n";
			}
			else {
				$line .= readline('... ')."\n";
			}
		}
	}

	public function instanceExists($name) {
		if($name)
			return clips_instance_exists($name);
		return false;
	}

	public function classExists($class) {
		if($class)
			return clips_class_exists($class);
		return false;
	}

	public function templateExists($template) {
		if($template)
			return clips_template_exists($template);
		return false;
	}

	public function assertFacts($data) {
		foreach($data as $fact) {
			if(is_object($fact)) { // Add the class as template for the object
				$this->template(get_class($fact));
			}
			$this->command('(assert '.$this->defineFact($fact).')');
		}
		return true;
	}


	public function defineFacts($name, $data) {
		$ret = array();
		$ret []= '(deffacts '.$name;
		foreach($data as $fact) {
			$ret []= $this->defineFact($fact);
		}
		$this->command(implode(' ', $ret).')');
	}

	public function defineFact($data) {
		$ret = array();
		if(is_array($data))  {
			if(!isset($data['__template__'])) { // This is a static fact
				$name = array_shift($data);
				$ret []= '('.$name;
				foreach($data as $d) {
					$ret []= $this->translate($d);
				}
			}
			else {
				return $this->defineFact((object) $data);
			}
		}
		else {
			$obj = $data;
			$name = get_class($obj);
			if(isset($obj->__template__)) {
				$name = $obj->__template__;
			}

			$ret []= '('.$name;
			foreach($obj as $key => $value) {
				if(strpos($key, '_') === 0) // Skip _ variables
					continue;
				$ret []= '('.$key;
				$ret []= $this->translate($value).')';
			}
		}
		return implode(' ', $ret).')';
	}

	public function agenda() {
		$this->command('(agenda)');
	}

	/**
	 * Execute the clips command
	 */
	public function command($command) {
		if(is_array($command)) {
			foreach($command as $c) {
				$this->command($c);
			}
			return;
		}
		clips_exec($command."\n"); // Add \n automaticly
	}

	public function queryFacts($name = null) {
		$arr = array();
		if(!$name)
			return clips_query_facts($arr);
		return clips_query_facts($arr, $name);
	}

	public function queryOneFact($name = null) {
		$ret = $this->queryFacts($name);
		if(count($ret))
			return $ret[0];
		return null;
	}

	public function lib($file) {
		if(file_exists($file)) {
			$this->command(file_get_contents($file));
			return true;
		}
		return false;
	}

	public function templates() {
		$this->command('(list-deftemplates)');
	}

	public function ci_load($file) {
		if(is_array($file)) {
			foreach($file as $f) {
				$this->ci_load($f);
			}
		}
		else {
			foreach(array(APPPATH, 'pinet/') as $p) {
				$path = FCPATH.$p.'config/rules/'.$file;
				if(file_exists($path)) {
					$this->load($path);
					break;
				}
			}
		}
	}

	/**
	 * Load and execute the clips rule file
	 */
	public function load($file) {
		if(is_array($file)) {
			foreach($file as $f) {
				$this->load($f);
			}
			return;
		}

		if(file_exists($file)) {
			clips_load($file);
		}
		else {
			trigger_error('The file '.$file.' is not found.');
		}
	}
}
