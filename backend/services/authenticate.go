package services

import (
	"errors"
	"fmt"
	"net/http"

	"time"

	jwt "github.com/dgrijalva/jwt-go"
	"github.com/satori/go.uuid"
	"github.com/zippyai/zippy/backend/data"
)

const (
	// AuthKey is the name of the auth cookie we use
	AuthKey = "auth"
)

var (
	// ErrUnAuthorized the user does not have permissions to access that resource
	ErrUnAuthorized = errors.New("user is not authorized")
	// ErrBadPassword the password that was given was invalid for auth
	ErrBadPassword = errors.New("password invalid for user")
	// ErrBadToken the token in the users cookies is invalid for auth
	ErrBadToken = errors.New("token is invalid for user")
	// ErrInvalidToken the token object is invalid
	ErrInvalidToken = errors.New("Token is invalid")
	// ErrNotAdmin returned if a user is not an admin
	ErrNotAdmin = errors.New("user is not an admin")

	authExpirationDuration = time.Hour * 14 * 24 // two weeks
)

// CreateUserAuth creates a new user for the auth information
func CreateUserAuth(store data.UserAuthentication, newAuth *data.UserAuth) (*data.User, error) {
	if newAuth.UserName == "" || newAuth.Password == "" {
		return nil, fmt.Errorf("new user auth invalid name: %s or pass: %s are empty", newAuth.UserName, newAuth.Password)
	}

	userID := uuid.NewV4().String()
	return store.CreateUserAuth(userID, newAuth.UserName, newAuth.Password)
}

// UpdateUserAuth updates users auth
func UpdateUserAuth(store data.UserStorage, auth *data.UserAuth) (*data.User, error) {
	user, err := store.FindUserByName(auth.UserName)
	if err != nil {
		return nil, err
	}

	return store.UpdateUserAuth(user.UserID, auth.UserName, auth.Password)
}

// AuthUser returns a specific error is the auth attempt was bad or nil if they're authorized
func AuthUser(storage data.UserStorage, auth *data.UserAuth) (*data.User, error) {
	user, err := storage.FindUserByName(auth.UserName)
	if err != nil {
		return nil, err
	}

	// check if a password or token was used for auth and check to see if they're valid
	if auth.Password != "" && user.Password != auth.Password {
		return nil, ErrBadPassword
	}

	return user, nil
}

// IsAdmin returns an error if a user is not found or is not an admin and nil if they're
// if not admin returns ErrNotAdmin
func IsAdmin(store data.UserStorage, userID string) error {
	user, err := store.FindUser(userID)
	if err != nil {
		return err
	}

	if !user.Admin {
		return ErrNotAdmin
	}

	return nil
}

// TokenFromCookie gets the jwt.Token from the value present in the cookie or error
func TokenFromCookie(cookie *http.Cookie, signingKey []byte) (*jwt.Token, error) {
	token, err := jwt.Parse(cookie.Value, func(token *jwt.Token) (interface{}, error) {
		if _, ok := token.Method.(*jwt.SigningMethodHMAC); !ok {
			return nil, fmt.Errorf("Invalid signing method")
		}

		return signingKey, nil
	})
	if err != nil {
		return nil, err
	}

	return token, nil
}

// ValidTokenFromRequest gets the cookie from the req and gets the tokens
func ValidTokenFromRequest(req *http.Request, signingKey []byte) (*jwt.Token, error) {
	// check to see if there is already a cookie present
	cookie, err := req.Cookie(AuthKey)
	if err != nil {
		return nil, err
	}

	token, err := TokenFromCookie(cookie, signingKey)
	if err != nil {
		return nil, err
	}

	if !token.Valid {
		return nil, ErrInvalidToken
	}

	return token, nil
}

// ValidateCookie parses the token from a cookie and checks to make sure it is valid error is nil if it is
func ValidateCookie(cookie *http.Cookie, signingKey []byte) error {
	token, err := TokenFromCookie(cookie, signingKey)
	if err != nil {
		return err
	}

	if !token.Valid {
		return ErrInvalidToken
	}

	return nil
}

// Claims is our jwt.Claims wrapper adding identifying information
type Claims struct {
	Identifier string `json:"identifier"`
	jwt.StandardClaims
}

// Valid is an implementation of the jwt.Claims interface
func (c Claims) Valid() error {
	return nil
}

// CreateAuthCookie creates a new cookie from a jwt signed token
func CreateAuthCookie(userID, issuer string, signingKey []byte) (*http.Cookie, error) {
	expireCookieTime := time.Now().Add(authExpirationDuration)

	signedKey, err := CreateSignedKey(userID, issuer, authExpirationDuration, signingKey)
	if err != nil {
		return nil, err
	}

	return &http.Cookie{
		Name:    AuthKey,
		Value:   signedKey,
		Expires: expireCookieTime,
		Path:    "/",
	}, nil
}

// CreateSignedKey creates a signed jwt key from the secret and claims
func CreateSignedKey(identifier, issuer string, expire time.Duration, signingKey []byte) (string, error) {
	var expireAt int64
	if expire > 0 {
		expireAt = time.Now().Add(expire).Unix()
	}

	token := createToken(identifier, issuer, expireAt)

	return token.SignedString(signingKey)
}

// identifier is either the vehicleID or username this is for and issuer is the host:port string for the server
func createToken(identifier, issuer string, expireAt int64) *jwt.Token {
	tClaims := Claims{
		identifier,
		jwt.StandardClaims{
			ExpiresAt: expireAt,
			Issuer:    issuer,
			IssuedAt:  time.Now().Unix(),
		},
	}

	return jwt.NewWithClaims(jwt.SigningMethodHS256, tClaims)
}

// PromoteUser upgrade user to admin
func PromoteUser(store data.UserAuthentication, userID string) error {
	return store.SetAdmin(userID, true)
}
