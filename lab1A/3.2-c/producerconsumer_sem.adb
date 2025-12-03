with Ada.Text_IO;
use Ada.Text_IO;

with Ada.Real_Time;
use Ada.Real_Time;

with Ada.Numerics.Discrete_Random;

with Semaphores;
use Semaphores;

procedure ProducerConsumer_Sem is
	
	N : constant Integer := 10; -- Number of produced and consumed tokens per task
	X : constant Integer := 3; -- Number of producers and consumer
		
	-- Buffer Definition
	Size: constant Integer := 4;
	type Index is mod Size;
	type Item_Array is array(Index) of Integer;
	B : Item_Array;
	In_Ptr, Out_Ptr : Index := 0;
	Count : Integer range 0 .. Size := 0;


   -- Random Delays
   subtype Delay_Interval is Integer range 50..250;
   package Random_Delay is new Ada.Numerics.Discrete_Random (Delay_Interval);
   use Random_Delay;
   G : Generator;
	
   -- => Complete code: Declation of Semaphores
	--    1. Semaphore 'NotFull' to indicate that buffer is not full
	--    2. Semaphore 'NotEmpty' to indicate that buffer is not empty
	--    3. Semaphore 'AtomicAccess' to ensure an atomic access to the buffer

   NotFull      : CountingSemaphore(Max => Size, Initial => Size);
   NotEmpty     : CountingSemaphore(Max => Size, Initial => 0);
   AtomicAccess : CountingSemaphore(Max => 1,    Initial => 1);


   task type Producer(Id : Positive);

   task type Consumer(Id : Positive);

   task body Producer is
      Next : Time;
			Value : Integer;
   begin
      Next := Clock;
      for I in 1..N loop
         -- => Complete Code: Write to Buffer
			Value := Random(G);
			NotFull.Wait; 
        		AtomicAccess.Wait; 

			B(In_Ptr) := Value;
      In_Ptr := In_Ptr + 1;
      Count := Count + 1;
      Put_Line("Producer " 
      	      & Integer'Image(Id) 
      	      & " put: " 
      	      & Integer'Image(Value)
      	      &" (Buffer count = " & Integer'Image(Count));


				 AtomicAccess.Signal;
         			 NotEmpty.Signal; 
         -- Next 'Release' in 50..250ms
         Next := Next + Milliseconds(Random(G));
         --delay until Next;
				 delay until Next;
      end loop;
   end;

   task body Consumer is
      Next : Time;
		  X : Integer;
   begin
      Next := Clock;
      for I in 1..N loop
         -- => Complete Code: Read from Buffer
		     NotEmpty.Wait;      
         AtomicAccess.Wait;

         X := B(Out_Ptr);
         Out_Ptr := Out_Ptr + 1;
         Count := Count - 1;
         Put_Line("Consumer " 
         	 & Integer'Image(Id) 
         	 & " got: " 
         	 & Integer'Image(X)
         	 &" (Buffer count = " & Integer'Image(Count));

         AtomicAccess.Signal;
         NotFull.Signal;
			-- Next 'Release' in 50..250ms
         Next := Next + Milliseconds(Random(G));
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
end ProducerConsumer_Sem;


