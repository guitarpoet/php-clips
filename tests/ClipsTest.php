<?php

require_once(dirname(__FILE__).'/../php/clips.php');

class ClipsTest extends PHPUnit_Framework_TestCase {
	public function setUp() {
		$this->clips = Clips::GetInstance();
	}

	public function tearDown() {
	}

	public function testContextAcess() {
		$this->assertNotNull($this->clips);
		$this->clips->hello = "world";
	}
}
