//gd class foobar extends Node2D
{


	var player1 = Rect2(Vector2(100,360),Vector2(25,100))
	var player2 = Rect2(Vector2(1180,360),Vector2(25,100))
	var P_SPEED:int = 500
	var B_SPEED:int = 1000

	var ball = Rect2(Vector2(640,360),Vector2(15,15))
	var ball_dir = Vector2(1,0)

	function _physics_process(delta){
		
		//Player one controls
		if(Input.is_action_pressed("w") && player1.position.y>0)
		{
			player1.position.y -= P_SPEED*delta
		}
		else if(Input.is_action_pressed("s") && player1.position.y<720)
			{
				player1.position.y += P_SPEED*delta
			}
		//Player two controls
		if(Input.is_action_pressed("ui_up") && player2.position.y>0)
			{
				player2.position.y -= P_SPEED*delta
			}
		else if(Input.is_action_pressed("ui_down") && player2.position.y<720)
			{
				player2.position.y += P_SPEED*delta
			}
		
		//Bal movement
		ball.position += ball_dir * B_SPEED * delta
		if(ball.intersects(player1,true))
		{		
			ball_dir = Vector2(1,0)
			var y_dist:float = player1.position.y - ball.position.y 
			var angle = (y_dist/player1.size.y)
			ball_dir = Vector2(1,0).rotated(angle)
		}
		else if(ball.intersects(player2,true))
		{
			var y_dist:float = player2.position.y - ball.position.y 
			var angle = y_dist/(player1.size.y*2)
			ball_dir = Vector2(-1,0).rotated(angle)
		}
			
		if(ball.position.y>720)
			{
				ball.position.y = 0
			}
		else if (ball.position.y<0)
			{
				ball.position.y = 720
			}
		if(ball.position.x>1280)
			{
				ball.position.x = 0
			}
		else if (ball.position.x<0)
			{
				ball.position.x = 1280
			}
		
		update()
	}


	function _draw()
	{
		draw_rect(player1,Color.blue)
		draw_rect(player2,Color.red)
		draw_rect(ball,Color.black)
	}
}
