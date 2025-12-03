with Ada.Text_IO;
use Ada.Text_IO;

with Ada.Real_Time;
use Ada.Real_Time;

with Ada.Numerics.Discrete_Random;

procedure ProducerConsumer_Rndzvs is
	
   N : constant Integer := 10; -- Number of produced and consumed tokens per task
	X : constant Integer := 3; -- Number of producers and consumers	
	
   -- Random Delays
   subtype Delay_Interval is Integer range 50..250;
   package Random_Delay is new Ada.Numerics.Discrete_Random (Delay_Interval);
   use Random_Delay;
   G : Generator;	
   task Buffer is
      entry Append(I : in Integer);
      entry Take(I : out Integer);
   end Buffer;

   task type Producer(Id : Positive);

   task type Consumer(Id : Positive);
   
   task body Buffer is
         Size: constant Integer := 4;
         type Index is mod Size;
         type Item_Array is array(Index) of Integer;
         B : Item_Array;
         In_Ptr, Out_Ptr : Index := 0;
				 Count: Integer range 0..Size := 0;
   begin
      loop
         select
				-- => Complete Code: Service Append
				 when Count < Size =>
							accept Append(I: in Integer) do
											B(In_Ptr) := I;
										  In_Ptr := In_Ptr + 1;
						  				Count := Count + 1; 
	    				end Append;
				 or
				-- => Complete Code: Service Take
				 when Count > 0 =>
							accept Take(I: out Integer) do
											I := B(Out_Ptr);
											Out_Ptr := Out_Ptr + 1;
											Count := Count - 1;
						  end Take;
				 or
				-- => Termination
         terminate;

				end select;
      end loop;
   end Buffer;
      
   task body Producer is
      Next : Time;
			Value : Integer;
   begin
      Next := Clock;
      for I in 1..N loop
			
         -- => Complete code: Write to X
				Value := Random(G);	
				Buffer.Append(Value);
        Put_Line("Producer " & Integer'Image(Id) & " Put :"&Integer'Image(Value));  

			 	-- Next 'Release' in 50..250ms
         Next := Next + Milliseconds(Random(G));
         delay until Next;
      end loop;
   end;

   task body Consumer is
      Next : Time;
      X : Integer;
   begin
      Next := Clock;
      for I in 1..N loop
         -- Complete Code: Read from X
		     Buffer.Take(X);
         Next := Next + Milliseconds(Random(G));
				 Put_Line("Consumer " & Integer'Image(Id) & " Get :"&Integer'Image(X));

				 delay until Next;
      end loop;
   end;
	
		 P1 : Producer(1);
		 P2 : Producer(2);
		 P3 : Producer(3);

		 C1 : Consumer(1);
		 C2 : Consumer(2);
		 C3 : Consumer(3);

begin -- main task
   null;
end ProducerConsumer_Rndzvs;

