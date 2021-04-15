package teleop

// UserView is the main view for sending user data to the web client
type UserView struct {
	UserID   string `json:"userID"`
	UserName string `json:"userName"`
}

// UserAuth is the login/register information sent up from the client
type UserAuth struct {
	UserName string `json:"userName"`
	Password string `json:"password"`
}
