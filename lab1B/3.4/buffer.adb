package body Buffer is
   protected body Watchdog_PO is

   procedure Feed is begin Fed := True; 
   end Feed; 
   
   function Is_Fed return Boolean is 
   begin 
   return Fed; 
   end Is_Fed; 
   
   procedure Reset is 
   begin Fed := False; 
   end Reset;

   end Watchdog_PO;
end Buffer;
