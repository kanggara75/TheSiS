<?php

namespace Database\Factories;

use Illuminate\Database\Eloquent\Factories\Factory;

class AppFactory extends Factory
{
	public function definition()
	{
		return [
			'user_id' => 1,
			'lat' => 1.6714782,
			'lon' => 101.4225598,
		];
	}
}
