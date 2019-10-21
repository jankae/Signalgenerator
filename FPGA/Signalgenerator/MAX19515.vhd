----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    10:01:38 10/21/2019 
-- Design Name: 
-- Module Name:    MAX19515 - Behavioral 
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

entity MAX19515 is
	 Generic (
		SampleEveryNthCLK : integer
	 );
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
			  -- See MAX19515 datasheet, page 25 for ADC signal names and timings
			  SAMPLE_CLK : out STD_LOGIC;
			  DCLK : in  STD_LOGIC;
           DATA : in  STD_LOGIC_VECTOR (9 downto 0);
			  
			  -- Processed ADC data
           OUT_A : out  STD_LOGIC_VECTOR (9 downto 0);
           OUT_B : out  STD_LOGIC_VECTOR (9 downto 0);
           NEW_SAMPLE : out  STD_LOGIC
    );
end MAX19515;

architecture Behavioral of MAX19515 is
	signal clk_cnt : integer range 0 to SampleEveryNthCLK/2 - 1;
	signal s_clk : STD_LOGIC;
	signal buf_A : std_logic_vector(9 downto 0);
begin

	SAMPLE_CLK <= s_clk;

	process(CLK, RESET)
	begin
		if(rising_edge(CLK)) then
			if(RESET = '1') then
				NEW_SAMPLE <= '0';
				clk_cnt <= 0;
				s_clk <= '0';
				buf_A <= (others => '0');
			else
				if(clk_cnt = SampleEveryNthCLK/2 - 1) then
					clk_cnt <= 0;
					-- toggle sample CLK
					s_clk <= not s_clk;
					-- store ADC data from one channel
					if(DCLK = '1') then
						buf_A <= DATA;
					else
						OUT_B <= DATA;
						OUT_A <= buf_A;
						-- both channels sampled, sample is complete
						NEW_SAMPLE <= '1';
					end if;
				else
					clk_cnt <= clk_cnt + 1;
					NEW_SAMPLE <= '0';
				end if;
			end if;
		end if;
	end process;

end Behavioral;

