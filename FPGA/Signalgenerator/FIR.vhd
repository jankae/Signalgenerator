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
use work.types.all;

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
		COEFF_ARRAY : in firarray(0 to Taps-1)
	);
end FIR;

architecture Behavioral of FIR is
	constant mult_latency : integer := 2;
	signal pipe : firarray(0 to Taps);
	signal mult_reg1 : firarray(0 to Taps-1);
	signal mult_reg2 : firarray(0 to Taps-1);
	signal mult_result : firarray(0 to Taps-1);
	signal mult_stage : integer range 0 to Multiplexed;
	signal data_latched : signed (11 downto 0);
	
	signal add_stage : integer range -mult_latency-1 to Multiplexed;
begin
	process(CLK)
		variable d : signed(11 downto 0);
	begin
		if(rising_edge(CLK)) then
			mult_result <= mult_reg2;
			mult_reg2 <= mult_reg1;
			if(RESET = '1') then
				for i in 0 to Taps loop
					pipe(i) <= (others=>'0');
				end loop;
				mult_stage <= 0;
				add_stage <= -mult_latency-1;
			else
				if ((NEW_DATA = '1') and (mult_stage = 0)) or mult_stage > 0 then
					if(NEW_DATA = '1') then
						data_latched <= DATA;
						d := DATA;
					else
						d := data_latched;
					end if;
					if add_stage = -mult_latency-1 then
						add_stage <= -mult_latency;
					end if;
					for i in 0 to (Taps/Multiplexed)-1 loop
						mult_reg1(mult_stage*(Taps/Multiplexed)+i) <= resize(d*COEFF_ARRAY(mult_stage*(Taps/Multiplexed)+i), 24)(22 downto 11);
					end loop;
					if(mult_stage < Multiplexed - 1) then
						mult_stage <= mult_stage + 1;
					else
						mult_stage <= 0;
					end if;
				end if;
				if add_stage >= -mult_latency then
					if(add_stage >= 0) then
						for i in 0 to (Taps/Multiplexed)-1 loop
							pipe(add_stage*(Taps/Multiplexed)+i) <= mult_result(add_stage*(Taps/Multiplexed)+i)
																				+pipe(add_stage*(Taps/Multiplexed)+i+1);
						end loop;
					end if;
					if(add_stage < Multiplexed - 1) then
						add_stage <= add_stage + 1;
					elsif (mult_stage > 0 and mult_stage <= mult_latency) then
						add_stage <= mult_stage-mult_latency;
					elsif NEW_DATA = '1' then
						add_stage <= -mult_latency;
					else
						add_stage <= -mult_latency-1;
					end if;
				end if;
			end if;
		end if;
	end process;
	
	output <= pipe(0);

end Behavioral;

