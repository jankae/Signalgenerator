--
--	Package File Template
--
--	Purpose: This package defines supplemental types, subtypes, 
--		 constants, and functions 
--
--   To use any of the example code shown below, uncomment the lines and modify as necessary
--

library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

package types is
	type coeffarray is array (natural range <>) of signed(11 downto 0);
	type addarray is array (natural range <>) of signed(11 downto 0);
end types;

