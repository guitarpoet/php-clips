<?php defined("BASEPATH") or exit("No direct script access allowed");

if(!class_exists('Annotation'))
	require_once(dirname(__FILE__).'/lib/addendum/annotations.php');

class ClipsMulti extends Annotation {}

class ClipsSymbol extends Annotation {
	public $value;
	// This annotation means this property of class is a symbol
	public function __construct($value = '') {
		$this->value = $value;
	}
} 

if(!function_exists('get_default')) {
	function get_default($arr, $key, $default = '') {
		if(is_object($arr))
			return isset($arr->$key)? $arr->$key: $default;
		if(is_array($arr))
			return isset($arr[$key])? $arr[$key]: $default;
		return $default;
	}
}

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
		if(function_exists('clips_load_rule')) { // If in CI, let's add ci_load function.
			$this->command('(deffunction ci_load (?file) (php_call "clips_load_rule" ?file))'); // Define the ci_load function
		}
	}

	private function defineClasses() {
		if(!$this->classExists('PHP_OBJECT')) // Define the PHP_OBJECT if needed
			$this->command($this->defineClass('PHP_OBJECT', 
				array('USER', 'OBJECT'), false, array()));
	}

	private function translate($var) {
		if($var === null)
			return 'nil';

		switch(gettype($var)) {
		case 'string':
			return '"'.addslashes($var).'"'; // Quote the string
		case 'boolean':
			return $var? 'TRUE' : 'FALSE';
		case 'array':
		case 'object':
			if(is_object($var) && get_class($var) == 'ClipsSymbol') {
				return $var->value;
			}
			// For array and object, let's make them multiple values
			$ret = array();
			foreach($var as $key => $value) {
				$ret []= $this->translate($value);
			}
			return implode(' ', $ret);
		}
		return $var;
	}

	/**
	 * Define the template in the clips for the php class
	 *
	 * @param $class
	 * 		The class name string for the php class
	 */
	public function template($class) {
		if(is_array($class)) {
			foreach($class as $c) {
				$this->template($c);
			}
			return true;
		}

		if(!$this->templateExists($class)) {
			$this->command($this->defineTemplate($class));
			return true;
		}
		return false;
	}

	//======================================================================
	//
	// Clips string manipulation methods
	//
	//======================================================================

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
			$reflection = new ReflectionAnnotatedClass($name);
			
			if(isset($obj->__template__)) {
				$name = $obj->__template__;
			}

			$ret []= '('.$name;
			foreach($obj as $key => $value) {
				if(strpos($key, '_') === 0) // Skip _ variables
					continue;
				$ret []= '('.$key;
				if($reflection->hasProperty($key)) {
					if($reflection->getProperty($key)->hasAnnotation('ClipsSymbol')) {
						$value = new ClipsSymbol($value);
					}
				}
				$ret []= $this->translate($value).')';
			}
		}
		return implode(' ', $ret).')';
	}

	public function defineInstance($name, $class = 'PHP_OBJECT', $args = array()) {
		$ret = array();
		$ret []= '(make-instance';
		$ret []= $name;
		$ret []= 'of';
		$ret []= $class;
		foreach($args as $key => $value) {
			$ret []= '('.$key;
			$ret []= $this->translate($value);
			$ret []= ')';
		}
		return implode(' ', $ret).')';
	}

	/**
	 * Define the template according to the class
	 */
	public function defineTemplate($class) {
		if(is_string($class) && class_exists($class)) {
			$reflection = new ReflectionAnnotatedClass($class);
			
			$ret = array();
			$ret []= '(deftemplate '.$class;
			foreach(get_class_vars($class) as $slot => $v) {
				if($reflection->getProperty($slot)->hasAnnotation('ClipsMulti')) {
					$ret []= '(multislot '.$slot.')';
				}
				else
					$ret []= '(slot '.$slot.')';
			} return implode(' ', $ret).')';
		}
		return false;
	}

	/**
	 * Define the template slot
	 */
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

	/**
	 * Define a clips class
	 */
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

	//======================================================================
	//
	// Clips Runtime commands
	//
	//======================================================================

	/**
	 * Reset the clips runtime
	 */
	public function reset() {
		$this->command('(reset)');
	}

	/**
	 * Clear all the defines, this will clear the clip's context also
	 */
	public function clear() {
		foreach(Clips::$context as $key => $value) {
			unset(Clips::$context[$key]);
		}
		$this->command('(clear)');
		$this->_init_base_support();
	}

	/**
	 * List the agenda of the clips
	 */
	public function agenda() {
		$this->command('(agenda)');
	}

	/**
	 * List all the templates in the clips context
	 */
	public function templates() {
		$this->command('(list-deftemplates)');
	}


	/**
	 * Show facts in the clips can use the args as filter
	 */
	public function facts() {
		$str = array('(facts');
		if(func_num_args()) {
			foreach(func_get_args() as $arg) {
				$str []= $arg;
			}
		}
		return $this->command(implode(' ', $str).')');
	}

	/**
	 * Run the clips context
	 */
	public function run() {
		$this->command('(run)');
	}

	//======================================================================
	//
	// Misc methods
	//
	//======================================================================

	public function __get($key) {
		if(isset(Clips::$context[$key])) {
			return Clips::$context[$key];
		}
		return $this->$key;
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
		if(func_num_args() > 1) { // We got multiple args call
			return $this->assertFacts(func_get_args());
		}

		if(!$data || !(is_object($data) || is_array($data))) { // The data must be array or object
			return false;
		}

		if(is_object($data) || // If the data is object
			!isset($data[0]) || // If the data is an hash
			is_string($data[0])) { // Or the first element of the data is string, let the data be an array
			$data = array($data);
		}

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
