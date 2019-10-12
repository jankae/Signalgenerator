----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    15:41:42 10/12/2019 
-- Design Name: 
-- Module Name:    FIR - Behavioral 
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
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;
package FIRfilter is
	type coeffarray is array (natural range <>) of signed(11 downto 0);
	type addarray is array (natural range <>) of signed(11 downto 0);
end package;

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use work.FIRfilter.all;

entity FIR is
	generic (
--		Datawidth : integer;
--		CoeffWidth : integer;
		Taps : integer;
		Multiplexed : integer
	);
	PORT (
		CLK : in std_logic;
		RESET : in std_logic;
		NEW_DATA : in std_logic;
		DATA : in signed(11 downto 0);
		OUTPUT : out signed(11 downto 0);
		COEFF_ARRAY : in coeffarray(0 to Taps-1)
	);
end FIR;

architecture Behavioral of FIR is
	signal pipe : addarray(0 to Taps);
	signal stage : integer range 0 to Multiplexed-1;
	signal data_latched : signed (11 downto 0);
	signal pipe_buffer : addarray(0 to Taps/Multiplexed - 1);
begin
	process(CLK)
	begin
		if(rising_edge(CLK)) then
			if(RESET = '1') then
				for i in 0 to Taps loop
					pipe(i) <= (others=>'0');
				end loop;
				stage <= 0;
			elsif (NEW_DATA = '1') and (stage = 0) then
				if(Multiplexed > 1) then
					for i in 0 to (Taps/Multiplexed)-1 loop
						pipe_buffer(i) <= pipe(i*Multiplexed+1)
									+resize(DATA*COEFF_ARRAY(i*Multiplexed), 24)(22 downto 11);
					end loop;
					stage <= 1;
				else
					for i in 0 to Taps-1 loop
						pipe(i) <= pipe(i+1)+resize(DATA*COEFF_ARRAY(i), 24)(22 downto 11);
					end loop;
				end if;
				data_latched <= DATA;
			elsif (stage > 0) then
				for i in 0 to (Taps/Multiplexed)-1 loop
					pipe(i*Multiplexed + stage) <= pipe(i*Multiplexed + stage+1)
								+resize(data_latched*COEFF_ARRAY(i*Multiplexed + stage), 24)(22 downto 11);
				end loop;
				if(stage < Multiplexed - 1) then
					stage <= stage + 1;
				else
					stage <= 0;
					for i in 0 to (Taps/Multiplexed)-1 loop
						pipe(i*Multiplexed) <= pipe_buffer(i);
					end loop;
				end if;
			end if;
		end if;
	end process;
	
	output <= pipe(0);

end Behavioral;

