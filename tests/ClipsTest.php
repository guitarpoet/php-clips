<?php

require_once(dirname(__FILE__).'/../php/clips.php');

class ClipsTest extends PHPUnit_Framework_TestCase {
	public function setUp() {
		$this->clips = new Clips();
	}

	public function tearDown() {
	}

	public function testContextAcess() {
		$this->assertNotNull($this->clips);
		$this->clips->hello = "world";
	}

	public function testDefineSlot() {
		echo $this->clips->defineSlot('hello', 'slot', '1',
			array(
				array(
					'type' => 'type',
					'value' => 'INTEGER'
				),
				array(
					'type' => 'range',
					'begin' => 1,
					'end' => 10
				),
				array(
					'type' => 'allowed-integers',
					'value' => '1|2|3|4|5|6'
				)
		));
		echo "\n";
	}

	public function testDefineClass() {
		echo $this->clips->defineClass('PHP_OBJECT', 
			array('USER', 'OBJECT'), false, array(
				'name', 'age', 
				array(
					'type' => 'multislot', 'name' => 'friends',
					'constraints' => array(
						array(
							'type' => 'range',
							'begin' => 1,
							'end' => 10
						),
						array(
							'type' => 'type',
							'value' => 'INTEGER'
						)
					)
				)
			)
		);
		echo "\n";
	}
}
