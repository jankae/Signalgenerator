----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    12:13:22 07/05/2019 
-- Design Name: 
-- Module Name:    PRBS - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity PRBS is
	generic ( W : integer);
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
           RESULT : out  STD_LOGIC_VECTOR (W-1 downto 0));
end PRBS;

architecture Behavioral of PRBS is
	signal vector : std_logic_vector(W-1 downto 0);
begin
	
	RESULT <= vector;

	process(CLK, RESET)
	begin
		if(RESET = '1') then
			vector <= (others => '1');
		elsif(rising_edge(CLK)) then
			vector(W-2 downto 0) <= vector(W-1 downto 1);
			vector(W-1) <= vector(0) xor vector(6) xor vector(8) xor vector(11);
		end if;
	end process;
end Behavioral;

